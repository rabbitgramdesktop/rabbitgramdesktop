/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"
#include "base/object_ptr.h"
#include "data/data_audio_msg_id.h"
#include "ui/effects/animations.h"

namespace Ui {
class FlatLabel;
class LabelSimple;
class IconButton;
class FilledSlider;
class MediaSlider;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

namespace Media::Player {
struct TrackState;
} // namespace Media::Player

namespace Data {
class DocumentMedia;
} // namespace Data

namespace Rabbit::Player {

class TopPillButton;
class CircleIconButton;
class PlayPauseButton;

class FullPlayerWidget final : public Ui::RpWidget {
public:
	FullPlayerWidget(
		QWidget *parent,
		not_null<Window::SessionController*> controller);
	~FullPlayerWidget();

	void showAnimated();
	void hideAnimated();
	[[nodiscard]] bool isClosing() const {
		return _hiding;
	}

	[[nodiscard]] rpl::producer<> closeRequests() const {
		return _closeRequests.events();
	}

	[[nodiscard]] rpl::producer<> hideFinished() const {
		return _hideFinished.events();
	}

	void setGoToItemCallback(Fn<void(not_null<const HistoryItem*>)> callback) {
		_goToItemCallback = std::move(callback);
	}
	void setBaseGeometry(const QRect &geometry);

protected:
	void paintEvent(QPaintEvent *e) override;
	void resizeEvent(QResizeEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

private:
	void setupControls();
	void setupSwipeBack();
	void updateControlsGeometry();
	void updatePlayPauseGeometry();
	void updateSongDetails();
	void updateTrackState(const Media::Player::TrackState &state);
	void updateRepeatButton();
	void updateOrderButton();
	void updateVolumeButton();
	void updatePlayPauseButton();
	void handleSeekProgress(float64 progress);
	void handleSeekFinished(float64 progress);
	void applyVolumeChange(float64 volume);
	void paintDefaultCoverIcon(QPainter &p, const QRect &rect);
	void updateSlide();

	const not_null<Window::SessionController*> _controller;

	object_ptr<TopPillButton> _closeButton;
	object_ptr<TopPillButton> _goToMessageButton;
	object_ptr<Ui::FlatLabel> _headerTitle;
	object_ptr<Ui::FlatLabel> _headerSubtitle;

	object_ptr<Ui::FlatLabel> _trackTitle;
	object_ptr<Ui::FlatLabel> _trackPerformer;

	object_ptr<Ui::FilledSlider> _progressSlider;
	object_ptr<Ui::LabelSimple> _timePassedLabel;
	object_ptr<Ui::LabelSimple> _timeTotalLabel;

	object_ptr<Ui::IconButton> _orderButton;
	object_ptr<CircleIconButton> _prevButton;
	object_ptr<PlayPauseButton> _playPauseButton;
	object_ptr<CircleIconButton> _nextButton;
	object_ptr<Ui::IconButton> _repeatButton;

	object_ptr<Ui::IconButton> _volumeButton;
	object_ptr<Ui::MediaSlider> _volumeSlider;

	std::shared_ptr<Data::DocumentMedia> _currentMedia;
	DocumentData *_currentDocument = nullptr;
	FullMsgId _currentContextId;
	Fn<void(not_null<const HistoryItem*>)> _goToItemCallback;

	QRect _coverRect;
	QRect _baseGeometry;
	crl::time _seekPositionMs = -1;
	crl::time _lastDurationMs = 0;
	bool _playing = false;
	bool _hiding = false;
	bool _hasCover = false;

	Ui::Animations::Simple _a_show;
	rpl::event_stream<> _closeRequests;
	rpl::event_stream<> _hideFinished;
	rpl::lifetime _trackLifetime;

	int _accumulatedDeltaX = 0;

};

} // namespace Rabbit::Player
