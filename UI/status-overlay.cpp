/******************************************************************************
    Copyright (C) 2026 by RDNA Cast contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
******************************************************************************/

#include "status-overlay.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>
#include <QShowEvent>
#include <QWindow>

#include <cstring>

StatusOverlayPosition StatusOverlayPositionFromString(const char *position)
{
	if (!position)
		return StatusOverlayPosition::BottomRight;

	if (strcmp(position, "TopLeft") == 0)
		return StatusOverlayPosition::TopLeft;
	if (strcmp(position, "TopRight") == 0)
		return StatusOverlayPosition::TopRight;
	if (strcmp(position, "BottomLeft") == 0)
		return StatusOverlayPosition::BottomLeft;

	return StatusOverlayPosition::BottomRight;
}

const char *StatusOverlayPositionToString(StatusOverlayPosition position)
{
	switch (position) {
	case StatusOverlayPosition::TopLeft:
		return "TopLeft";
	case StatusOverlayPosition::TopRight:
		return "TopRight";
	case StatusOverlayPosition::BottomLeft:
		return "BottomLeft";
	case StatusOverlayPosition::BottomRight:
	default:
		return "BottomRight";
	}
}

int ClampStatusOverlayOpacity(int opacity)
{
	if (opacity < 20)
		return 20;
	if (opacity > 100)
		return 100;
	return opacity;
}

QRect StatusOverlayGeometryForScreen(const QRect &screenGeometry, const QSize &overlaySize,
				     StatusOverlayPosition position, int margin)
{
	int x = screenGeometry.left() + margin;
	int y = screenGeometry.top() + margin;

	if (position == StatusOverlayPosition::TopRight || position == StatusOverlayPosition::BottomRight)
		x = screenGeometry.right() - overlaySize.width() - margin + 1;

	if (position == StatusOverlayPosition::BottomLeft || position == StatusOverlayPosition::BottomRight)
		y = screenGeometry.bottom() - overlaySize.height() - margin + 1;

	x = qMax(screenGeometry.left(), x);
	y = qMax(screenGeometry.top(), y);

	return QRect(QPoint(x, y), overlaySize);
}

OBSStatusOverlay::OBSStatusOverlay(QWidget *owner_) : QWidget(nullptr), owner(owner_)
{
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
		       Qt::WindowDoesNotAcceptFocus);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_NativeWindow);
	setFixedSize(156, 44);
	setWindowOpacity(opacity / 100.0);

	ApplyCaptureExclusionProperty();

	flashTimer.setSingleShot(true);
	connect(&flashTimer, &QTimer::timeout, this, [this] {
		flashText.clear();
		RefreshVisibility();
		update();
	});
}

void OBSStatusOverlay::SetOverlayEnabled(bool enabled_)
{
	enabled = enabled_;
	RefreshVisibility();
}

void OBSStatusOverlay::SetOverlayPosition(StatusOverlayPosition position_)
{
	position = position_;
	Reposition();
}

void OBSStatusOverlay::SetOverlayOpacity(int opacity_)
{
	opacity = ClampStatusOverlayOpacity(opacity_);
	setWindowOpacity(opacity / 100.0);
}

void OBSStatusOverlay::SetStreaming(bool active)
{
	streaming = active;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetRecording(bool active)
{
	recording = active;
	if (!recording)
		recordingPaused = false;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetRecordingPaused(bool paused)
{
	recordingPaused = paused;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetReplayBuffer(bool active)
{
	replayBuffer = active;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::FlashAction(const QString &text)
{
	if (text.isEmpty())
		return;

	flashText = text;
	flashTimer.start(1200);
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::RefreshVisibility()
{
	const bool shouldShow = enabled && (!StatusText().isEmpty() || flashTimer.isActive());

	if (shouldShow) {
		Reposition();
		if (!isVisible())
			show();
		raise();
	} else if (isVisible()) {
		hide();
	}
}

void OBSStatusOverlay::paintEvent(QPaintEvent *event)
{
	(void)event;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const QRectF bounds = rect().adjusted(1, 1, -1, -1);
	painter.setPen(QColor(255, 255, 255, 38));
	painter.setBrush(QColor(13, 15, 18, 235));
	painter.drawRoundedRect(bounds, 8, 8);

	const QColor accent = StatusColor();
	painter.setPen(Qt::NoPen);
	painter.setBrush(accent);
	painter.drawEllipse(QRectF(14, 16, 12, 12));

	QFont textFont = font();
	textFont.setBold(true);
	textFont.setPointSize(10);
	painter.setFont(textFont);
	painter.setPen(QColor(246, 248, 250));
	painter.drawText(QRect(34, 0, width() - 44, height()), Qt::AlignVCenter | Qt::AlignLeft, StatusText());
}

void OBSStatusOverlay::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	ApplyCaptureExclusionProperty();
	Reposition();
}

QString OBSStatusOverlay::StatusText() const
{
	if (!flashText.isEmpty())
		return flashText;

	if (streaming && recording)
		return QStringLiteral("LIVE + REC");
	if (streaming)
		return QStringLiteral("LIVE");
	if (recording && recordingPaused)
		return QStringLiteral("PAUSED");
	if (recording)
		return QStringLiteral("REC");
	if (replayBuffer)
		return QStringLiteral("REPLAY");

	return QString();
}

QColor OBSStatusOverlay::StatusColor() const
{
	if (!flashText.isEmpty())
		return QColor(255, 184, 77);
	if (recordingPaused)
		return QColor(255, 184, 77);
	if (streaming || recording)
		return QColor(255, 72, 72);
	if (replayBuffer)
		return QColor(75, 195, 255);

	return QColor(115, 209, 116);
}

void OBSStatusOverlay::Reposition()
{
	QScreen *screen = nullptr;

	if (owner && owner->windowHandle())
		screen = owner->windowHandle()->screen();
	if (!screen)
		screen = QGuiApplication::primaryScreen();
	if (!screen)
		return;

	setGeometry(StatusOverlayGeometryForScreen(screen->availableGeometry(), size(), position, 24));
}

void OBSStatusOverlay::ApplyCaptureExclusionProperty()
{
	winId();

	if (QWindow *window = windowHandle())
		window->setProperty("forceHideFromCapture", true);
}
