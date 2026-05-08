#ifdef OBS_AMD_LITE

#include "update-lite-amd.hpp"
#include "window-basic-main.hpp"
#include <obs-app.hpp>
#include <qt-wrappers.hpp>
#include <util/platform.h>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "version.lib")

/* Query the file-version of the AMD AMF runtime DLL (amfrt64.dll), which is
 * installed alongside the Adrenalin driver. Returns empty string if the DLL
 * isn't present or the version data can't be read. */
static QString GetAMFRuntimeVersion()
{
	const char *dllName = "amfrt64.dll";
	DWORD handle = 0;
	DWORD size = GetFileVersionInfoSizeA(dllName, &handle);
	if (size == 0)
		return QString();
	QByteArray buf(static_cast<int>(size), 0);
	if (!GetFileVersionInfoA(dllName, handle, size, buf.data()))
		return QString();
	VS_FIXEDFILEINFO *info = nullptr;
	UINT infoSize = 0;
	if (!VerQueryValueA(buf.constData(), "\\", reinterpret_cast<LPVOID *>(&info), &infoSize) || !info)
		return QString();
	return QStringLiteral("%1.%2.%3.%4")
		.arg(HIWORD(info->dwFileVersionMS))
		.arg(LOWORD(info->dwFileVersionMS))
		.arg(HIWORD(info->dwFileVersionLS))
		.arg(LOWORD(info->dwFileVersionLS));
}
#else
static QString GetAMFRuntimeVersion()
{
	return QString();
}
#endif

#include <curl/curl.h>
#include <util/curl/curl-helper.h>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QApplication>
#include <QTimer>
#include <QFileInfo>

#include <json11.hpp>
#include "remote-text.hpp"

using namespace json11;

/* ============================================================
 * GKUpdateThread — Check GitHub releases API for new version
 * ============================================================ */

GKUpdateThread::GKUpdateThread(bool manual, QObject *parent)
	: QThread(parent),
	  manualCheck(manual)
{
}

static bool IsNewerVersion(const QString &latest, const QString &current)
{
	QStringList latestParts = latest.split('.');
	QStringList currentParts = current.split('.');

	for (int i = 0; i < 3; i++) {
		int l = (i < latestParts.size()) ? latestParts[i].toInt() : 0;
		int c = (i < currentParts.size()) ? currentParts[i].toInt() : 0;
		if (l > c)
			return true;
		if (l < c)
			return false;
	}
	return false;
}

void GKUpdateThread::run()
{
	/* Fetch latest release from GitHub API */
	std::string output;
	std::string error;
	long responseCode = 0;
	std::string versionHeader = "User-Agent: RDNACast/";
	versionHeader += GK_OBS_LITE_VERSION;

	std::vector<std::string> extraHeaders;
	extraHeaders.push_back(versionHeader);
	extraHeaders.push_back("Accept: application/vnd.github.v3+json");
	bool success = GetRemoteFile(GK_OBS_LITE_RELEASES_API, output, error, &responseCode, nullptr, std::string(),
				     nullptr, extraHeaders, nullptr, 15, false);

	if (!success || responseCode != 200) {
		if (manualCheck) {
			QString errMsg = QString("Failed to check for updates (HTTP %1)").arg(responseCode);
			if (!error.empty())
				errMsg += QString(": %1").arg(QString::fromStdString(error));
			emit UpdateError(errMsg);
		}
		return;
	}

	/* Parse JSON response */
	std::string jsonError;
	Json json = Json::parse(output, jsonError);
	if (!jsonError.empty()) {
		if (manualCheck)
			emit UpdateError(QString("Failed to parse update response: %1").arg(QString::fromStdString(jsonError)));
		return;
	}

	/* Extract version from tag_name (strip leading 'v' if present) */
	std::string tagName = json["tag_name"].string_value();
	if (tagName.empty()) {
		if (manualCheck)
			emit UpdateError("No releases found.");
		return;
	}

	QString latestVersion = QString::fromStdString(tagName);
	if (latestVersion.startsWith('v') || latestVersion.startsWith('V'))
		latestVersion = latestVersion.mid(1);

	QString currentVersion = GK_OBS_LITE_VERSION;

	if (!IsNewerVersion(latestVersion, currentVersion)) {
		emit NoUpdate(manualCheck);
		return;
	}

	/* Find download URL — prefer .exe installer, fallback to first asset */
	QString downloadUrl;
	const Json::array &assets = json["assets"].array_items();
	for (const Json &asset : assets) {
		std::string name = asset["name"].string_value();
		std::string url = asset["browser_download_url"].string_value();
		if (name.find(".exe") != std::string::npos || name.find("Setup") != std::string::npos) {
			downloadUrl = QString::fromStdString(url);
			break;
		}
	}
	if (downloadUrl.isEmpty() && !assets.empty()) {
		downloadUrl = QString::fromStdString(assets[0]["browser_download_url"].string_value());
	}

	QString releaseNotes = QString::fromStdString(json["body"].string_value());

	emit UpdateAvailable(latestVersion, downloadUrl, releaseNotes);
}

/* ============================================================
 * GKUpdateDialog — Download and apply update
 * ============================================================ */

GKUpdateDialog::GKUpdateDialog(QWidget *parent, const QString &ver, const QString &url, const QString &notes)
	: QDialog(parent),
	  version(ver),
	  downloadUrl(url)
{
	setWindowTitle("Update Available — RDNA Cast");
	setMinimumWidth(450);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto *layout = new QVBoxLayout(this);

	auto *titleLabel = new QLabel(QString("<h2>RDNA Cast %1 is available!</h2>"
					      "<p>You are currently running version %2.</p>")
					      .arg(ver, GK_OBS_LITE_VERSION));
	titleLabel->setWordWrap(true);
	layout->addWidget(titleLabel);

	if (!notes.isEmpty()) {
		auto *notesLabel = new QLabel("<b>Release Notes:</b>");
		layout->addWidget(notesLabel);

		auto *notesBrowser = new QTextBrowser();
		notesBrowser->setMarkdown(notes);
		notesBrowser->setMaximumHeight(200);
		notesBrowser->setOpenExternalLinks(true);
		layout->addWidget(notesBrowser);
	}

	statusLabel = new QLabel("Ready to download.");
	layout->addWidget(statusLabel);

	progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	progressBar->setValue(0);
	progressBar->setVisible(false);
	layout->addWidget(progressBar);

	auto *buttonLayout = new QHBoxLayout();
	updateButton = new QPushButton("Download && Install");
	cancelButton = new QPushButton("Later");
	buttonLayout->addStretch();
	buttonLayout->addWidget(updateButton);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(updateButton, &QPushButton::clicked, this, &GKUpdateDialog::StartDownload);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	if (downloadUrl.isEmpty()) {
		updateButton->setEnabled(false);
		statusLabel->setText("No installer found in this release. Check GitHub manually.");
	}
}

GKUpdateDialog::~GKUpdateDialog()
{
	/* Signal any running download thread to stop */
	cancelRequested = true;
	if (dlThread && dlThread->isRunning()) {
		dlThread->wait(5000);
	}
}

/* Curl write callback — writes to QFile, checks cancel flag */
struct GKDownloadContext {
	QFile *file;
	volatile bool *cancel;
};

static size_t curl_file_write(void *ptr, size_t size, size_t nmemb, void *data)
{
	auto *ctx = static_cast<GKDownloadContext *>(data);
	if (*ctx->cancel)
		return 0; /* Returning 0 aborts the transfer */
	return ctx->file->write(static_cast<const char *>(ptr), size * nmemb);
}

/* Curl progress callback — reports progress to UI thread */
struct GKProgressContext {
	GKUpdateDialog *dialog;
	volatile bool *cancel;
};

static int curl_progress_cb(void *data, curl_off_t dltotal, curl_off_t dlnow,
			    curl_off_t /*ultotal*/, curl_off_t /*ulnow*/)
{
	auto *ctx = static_cast<GKProgressContext *>(data);
	if (*ctx->cancel)
		return 1; /* Non-zero aborts transfer */
	if (dltotal > 0) {
		int percent = (int)((dlnow * 100) / dltotal);
		double dlMB = dlnow / 1048576.0;
		double totalMB = dltotal / 1048576.0;
		QMetaObject::invokeMethod(ctx->dialog, "OnDownloadProgress",
					  Qt::QueuedConnection,
					  Q_ARG(int, percent),
					  Q_ARG(double, dlMB),
					  Q_ARG(double, totalMB));
	}
	return 0;
}

void GKUpdateDialog::StartDownload()
{
	updateButton->setEnabled(false);
	cancelButton->setText("Cancel");
	progressBar->setVisible(true);
	progressBar->setValue(0);
	statusLabel->setText("Downloading...");
	cancelRequested = false;

	/* Connect cancel button to set cancel flag */
	disconnect(cancelButton, nullptr, nullptr, nullptr);
	connect(cancelButton, &QPushButton::clicked, this, [this]() {
		cancelRequested = true;
		statusLabel->setText("Cancelling...");
		cancelButton->setEnabled(false);
	});

	/* Clean up any previous download */
	QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	QString filename = QUrl(downloadUrl).fileName();
	if (filename.isEmpty())
		filename = "RDNACast_Setup.exe";
	installerPath = tempDir + "/" + filename;
	QFile::remove(installerPath);

	QString url = downloadUrl;
	QString path = installerPath;

	dlThread = QThread::create([this, url, path]() {
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly)) {
			QMetaObject::invokeMethod(this, "OnDownloadError",
						  Qt::QueuedConnection,
						  Q_ARG(QString, QString("Cannot write to %1").arg(path)));
			return;
		}

		CURL *curl = curl_easy_init();
		if (!curl) {
			file.close();
			file.remove();
			QMetaObject::invokeMethod(this, "OnDownloadError",
						  Qt::QueuedConnection,
						  Q_ARG(QString, QString("Failed to initialize downloader.")));
			return;
		}

		GKDownloadContext dlCtx{&file, &cancelRequested};
		GKProgressContext progCtx{this, &cancelRequested};

		std::string urlStr = url.toStdString();
		curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_file_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dlCtx);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curl_progress_cb);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progCtx);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "RDNACast/" GK_OBS_LITE_VERSION);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_obs_set_revoke_setting(curl);

		CURLcode res = curl_easy_perform(curl);
		file.close();
		curl_easy_cleanup(curl);

		if (cancelRequested) {
			QFile::remove(path);
			return;
		}

		if (res != CURLE_OK) {
			QFile::remove(path);
			QMetaObject::invokeMethod(this, "OnDownloadError",
						  Qt::QueuedConnection,
						  Q_ARG(QString, QString("Download failed: %1").arg(curl_easy_strerror(res))));
			return;
		}

		/* Verify file is not empty/corrupt */
		QFileInfo fi(path);
		if (!fi.exists() || fi.size() < 1024 * 100) { /* < 100KB is definitely corrupt */
			QFile::remove(path);
			QMetaObject::invokeMethod(this, "OnDownloadError",
						  Qt::QueuedConnection,
						  Q_ARG(QString, QString("Downloaded file is too small (%1 bytes). Possibly corrupt.").arg(fi.size())));
			return;
		}

		QMetaObject::invokeMethod(this, "OnDownloadFinished", Qt::QueuedConnection);
	});

	connect(dlThread, &QThread::finished, dlThread, &QObject::deleteLater);
	connect(dlThread, &QThread::finished, this, [this]() { dlThread = nullptr; });
	dlThread->start();
}

void GKUpdateDialog::OnDownloadProgress(int percent, double downloadedMB, double totalMB)
{
	progressBar->setValue(percent);
	statusLabel->setText(QString("Downloading: %1 MB / %2 MB (%3%)")
				    .arg(downloadedMB, 0, 'f', 1)
				    .arg(totalMB, 0, 'f', 1)
				    .arg(percent));
}

void GKUpdateDialog::OnDownloadFinished()
{
	progressBar->setValue(100);
	statusLabel->setText("Download complete. Launching installer...");
	updateButton->setEnabled(false);
	cancelButton->setEnabled(false);

	/* Launch installer after a short delay, then exit OBS cleanly.
	 * We use a PowerShell helper script (like MyLocalBackup) that:
	 * 1. Waits for OBS to fully exit
	 * 2. Then launches the installer
	 * This avoids the crash from QApplication::quit() racing with
	 * audio/video thread shutdown. */
	QTimer::singleShot(500, this, [this]() {
		/* Create a helper script that waits for us to exit, then runs installer */
		QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		QString scriptPath = tempDir + "/rdnacast_update.bat";
		QFile script(scriptPath);
		if (script.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QString bat = QString(
				"@echo off\r\n"
				"timeout /t 3 /nobreak >nul\r\n"
				"start \"\" \"%1\"\r\n"
				"del \"%2\"\r\n"
			).arg(installerPath.replace("/", "\\"), scriptPath.replace("/", "\\"));
			script.write(bat.toUtf8());
			script.close();

			QProcess::startDetached("cmd.exe", QStringList() << "/c" << scriptPath);
		} else {
			/* Fallback: launch installer directly */
			QProcess::startDetached(installerPath, QStringList());
		}

		/* Close OBS properly — use the main window's close path */
		OBSBasic *main = OBSBasic::Get();
		if (main) {
			main->ForceClose();
		} else {
			QApplication::quit();
		}
	});
}

void GKUpdateDialog::OnDownloadError(const QString &error)
{
	statusLabel->setText("Download failed: " + error);
	updateButton->setEnabled(true);
	updateButton->setText("Retry");
	progressBar->setVisible(false);
	cancelButton->setText("Later");
	cancelButton->setEnabled(true);
	disconnect(cancelButton, nullptr, nullptr, nullptr);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

/* ============================================================
 * GKAboutDialog - About + license notices
 * ============================================================ */

GKAboutDialog::GKAboutDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("About RDNA Cast");
	setFixedSize(480, 420);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto *layout = new QVBoxLayout(this);

	QString amfVersion = GetAMFRuntimeVersion();
	QString amfLine = amfVersion.isEmpty()
				  ? QStringLiteral("<p><b>AMD AMF runtime:</b> not detected</p>")
				  : QStringLiteral("<p><b>AMD AMF runtime:</b> %1</p>").arg(amfVersion);

	auto *titleLabel = new QLabel(
		"<div style='text-align:center;'>"
		"<h1 style='color:#e63946;'>RDNA Cast</h1>"
		"<p style='font-size:14px;'>Version " GK_OBS_LITE_VERSION " (64-bit)</p>"
		"<p>High-performance streaming for AMD Radeon. Gaming unleashed.</p>"
		"<p><b>Developer:</b> George Karagioules</p>"
		"<p><b>Based on:</b> OBS Studio 31.0.3 by the OBS Project</p>"
		+ amfLine +
		"<p style='font-size:11px;color:#888;'>Not affiliated with, endorsed by, or sponsored by "
		"Advanced Micro Devices, Inc. or the OBS Project. &quot;AMD&quot;, &quot;Radeon&quot;, and "
		"&quot;RDNA&quot; are trademarks of Advanced Micro Devices, Inc.</p>"
		"<hr>"
		"</div>");
	titleLabel->setWordWrap(true);
	titleLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(titleLabel);

	auto *eulaLabel = new QLabel("<b>Licenses</b>");
	layout->addWidget(eulaLabel);

	auto *eulaBrowser = new QTextBrowser();
	eulaBrowser->setPlainText(
		"RDNA Cast is an OBS Studio fork. The combined application and "
		"binary distributions are redistributed under the GNU General "
		"Public License version 2.0 or, at your option, any later version "
		"(GPL-2.0-or-later).\n\n"
		"The complete GPL text is included in the installed COPYING file "
		"and in the source repository.\n\n"
		"Original RDNA Cast additions are also offered under the MIT "
		"License by George Karagioules. That MIT grant applies only to "
		"original RDNA Cast additions and does not relicense OBS Studio or "
		"the combined OBS-derived application.\n\n"
		"MIT License - RDNA Cast original additions\n\n"
		"Copyright (c) 2026 George Karagioules\n\n"
		"Permission is hereby granted, free of charge, to any person obtaining a copy "
		"of the original RDNA Cast additions and associated documentation files "
		"(the \"Software\"), to deal in the Software without restriction, including "
		"without limitation the rights to use, copy, modify, merge, publish, "
		"distribute, sublicense, and/or sell copies of the Software, and to permit "
		"persons to whom the Software is furnished to do so, subject to the "
		"following conditions:\n\n"
		"The above copyright notice and this permission notice shall be included in all "
		"copies or substantial portions of the Software.\n\n"
		"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
		"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
		"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
		"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
		"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
		"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
		"SOFTWARE.\n\n"
		"---\n\n"
		"Bundled third-party components retain their own licenses. See "
		"THIRD_PARTY_LICENSES.md installed alongside the application.\n\n"
		"OBS Studio: Copyright (C) Hugh Bailey and contributors.\n"
		"AMD AMF SDK: Copyright (c) Advanced Micro Devices, Inc.\n\n"
		"Source: https://github.com/georgekgr12/RDNACast");
	eulaBrowser->setReadOnly(true);
	layout->addWidget(eulaBrowser);

	auto *closeButton = new QPushButton("Close");
	closeButton->setDefault(true);
	auto *btnLayout = new QHBoxLayout();
	btnLayout->addStretch();
	btnLayout->addWidget(closeButton);
	layout->addLayout(btnLayout);

	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

#endif /* OBS_AMD_LITE */
