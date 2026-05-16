/******************************************************************************
    Copyright (C) 2026 by RDNA Cast contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
******************************************************************************/

#pragma once

#include <QPointer>
#include <QTimer>
#include <QWidget>

enum class StatusOverlayPosition {
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

StatusOverlayPosition StatusOverlayPositionFromString(const char *position);
const char *StatusOverlayPositionToString(StatusOverlayPosition position);
int ClampStatusOverlayOpacity(int opacity);
QRect StatusOverlayGeometryForScreen(const QRect &screenGeometry, const QSize &overlaySize,
				     StatusOverlayPosition position, int margin);

class OBSStatusOverlay : public QWidget {
public:
	explicit OBSStatusOverlay(QWidget *owner);

	void SetOverlayEnabled(bool enabled);
	void SetOverlayPosition(StatusOverlayPosition position);
	void SetOverlayOpacity(int opacity);
	void SetStreaming(bool active);
	void SetRecording(bool active);
	void SetRecordingPaused(bool paused);
	void SetReplayBuffer(bool active);
	void FlashAction(const QString &text);
	void RefreshVisibility();

protected:
	void paintEvent(QPaintEvent *event) override;
	void showEvent(QShowEvent *event) override;

private:
	QString StatusText() const;
	QColor StatusColor() const;
	void Reposition();
	void ApplyCaptureExclusionProperty();

	QPointer<QWidget> owner;
	QTimer flashTimer;
	QString flashText;
	StatusOverlayPosition position = StatusOverlayPosition::BottomRight;
	int opacity = 85;
	bool enabled = false;
	bool streaming = false;
	bool recording = false;
	bool recordingPaused = false;
	bool replayBuffer = false;
};
