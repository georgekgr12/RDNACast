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
#include <QStringList>
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
	setAttribute(Qt::WA_ShowWithoutActivating);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
	setFixedSize(186, 28);

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
	if (enabled == enabled_)
		return;

	enabled = enabled_;
	RefreshVisibility();
}

void OBSStatusOverlay::SetOverlayPosition(StatusOverlayPosition position_)
{
	if (position == position_)
		return;

	position = position_;
	Reposition();
}

void OBSStatusOverlay::SetStreaming(bool active)
{
	if (streaming == active)
		return;

	streaming = active;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetRecording(bool active)
{
	if (recording == active)
		return;

	recording = active;
	if (!recording)
		recordingPaused = false;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetRecordingPaused(bool paused)
{
	if (recordingPaused == paused)
		return;

	recordingPaused = paused;
	RefreshVisibility();
	update();
}

void OBSStatusOverlay::SetReplayBuffer(bool active)
{
	if (replayBuffer == active)
		return;

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
	const bool shouldShow = enabled && HasActiveStatus();

	if (shouldShow) {
		Reposition();
		if (!isVisible()) {
			show();
			raise();
		}
	} else if (isVisible()) {
		hide();
	}
}

void OBSStatusOverlay::paintEvent(QPaintEvent *event)
{
	(void)event;

	QPainter painter(this);

	const QColor accent = StatusColor();
	painter.fillRect(rect(), QColor(12, 13, 15));
	painter.fillRect(QRect(0, 0, 4, height()), accent);
	painter.setPen(QColor(255, 255, 255, 80));
	painter.drawRect(rect().adjusted(0, 0, -1, -1));

	QFont textFont = font();
	textFont.setBold(true);
	textFont.setPointSize(10);
	painter.setFont(textFont);
	painter.setPen(QColor(246, 248, 250));
	painter.drawText(QRect(13, 0, width() - 18, height()), Qt::AlignVCenter | Qt::AlignLeft, StatusText());
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

	QStringList states;

	if (streaming)
		states.push_back(QStringLiteral("LIVE"));
	if (recording && recordingPaused)
		states.push_back(QStringLiteral("PAUSED"));
	else if (recording)
		states.push_back(QStringLiteral("REC"));
	if (replayBuffer)
		states.push_back(QStringLiteral("REPLAY"));

	return states.join(QStringLiteral(" + "));
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

bool OBSStatusOverlay::HasActiveStatus() const
{
	return streaming || recording || replayBuffer || !flashText.isEmpty();
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

	const QRect targetGeometry = StatusOverlayGeometryForScreen(screen->availableGeometry(), size(), position, 24);
	if (geometry() != targetGeometry)
		setGeometry(targetGeometry);
}

void OBSStatusOverlay::ApplyCaptureExclusionProperty()
{
	winId();

	if (QWindow *window = windowHandle())
		window->setProperty("forceHideFromCapture", true);
}
