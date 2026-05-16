#pragma once

#ifdef OBS_AMD_LITE

#include <QThread>
#include <QDialog>
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextBrowser>

/* ============================================================
 * OBS Lite AMD Edition — Assisted Update System
 * Fetches latest release from GitHub, downloads installer,
 * runs it, and restarts the app.
 * ============================================================ */

#define GK_OBS_LITE_VERSION "0.7.17"
#define GK_OBS_LITE_PRODUCT_NAME "RDNA Cast"
#define GK_OBS_LITE_RELEASES_REPO "karagioules/RDNA_Cast"
#define GK_OBS_LITE_RELEASES_API "https://api.github.com/repos/" GK_OBS_LITE_RELEASES_REPO "/releases/latest"

/* --- Update Check Thread --- */

class GKUpdateThread : public QThread {
	Q_OBJECT

public:
	explicit GKUpdateThread(bool manual, QObject *parent = nullptr);

signals:
	void UpdateAvailable(const QString &version, const QString &downloadUrl, const QString &releaseNotes);
	void NoUpdate(bool manual);
	void UpdateError(const QString &error);

protected:
	void run() override;

private:
	bool manualCheck;
};

/* --- Download + Apply Dialog --- */

class GKUpdateDialog : public QDialog {
	Q_OBJECT

public:
	GKUpdateDialog(QWidget *parent, const QString &version, const QString &downloadUrl,
		       const QString &releaseNotes);
	~GKUpdateDialog();

public slots:
	void StartDownload();
	void OnDownloadFinished();
	void OnDownloadError(const QString &error);
	void OnDownloadProgress(int percent, double downloadedMB, double totalMB);

private:
	QString version;
	QString downloadUrl;
	QString installerPath;
	QLabel *statusLabel;
	QProgressBar *progressBar;
	QPushButton *updateButton;
	QPushButton *cancelButton;
	QThread *dlThread = nullptr;
	volatile bool cancelRequested = false;
};

/* --- About / License Dialog --- */

class GKAboutDialog : public QDialog {
	Q_OBJECT

public:
	explicit GKAboutDialog(QWidget *parent = nullptr);
};

#endif /* OBS_AMD_LITE */
