/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"
#include "base/object_ptr.h"
#include "base/unique_qptr.h"
#include "data/data_audio_msg_id.h"
#include "ui/effects/animations.h"
#include "rabbit/ui/player/cover_theme.h"

namespace Ui {
class FlatLabel;
class LabelSimple;
class IconButton;
class PopupMenu;
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
class SpeedPillButton;
class QueuePillButton;
class ExpressiveSlider;
class QueuePanel;

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
	void updateCoverTheme();
	void applyCoverTheme(const CoverTheme &theme);
	void transitionToTheme(const CoverTheme &targetTheme, bool animated = true);
	void showMoreMenu();
	void toggleQueue();

	const not_null<Window::SessionController*> _controller;

	object_ptr<TopPillButton> _closeButton;
	object_ptr<CircleIconButton> _moreButton;
	object_ptr<Ui::FlatLabel> _headerTitle;
	object_ptr<Ui::FlatLabel> _headerSubtitle;

	object_ptr<Ui::FlatLabel> _trackTitle;
	object_ptr<Ui::FlatLabel> _trackPerformer;

	object_ptr<ExpressiveSlider> _progressSlider;
	object_ptr<Ui::FlatLabel> _timePassedLabel;
	object_ptr<Ui::FlatLabel> _timeTotalLabel;

	object_ptr<Ui::IconButton> _orderButton;
	object_ptr<CircleIconButton> _prevButton;
	object_ptr<PlayPauseButton> _playPauseButton;
	object_ptr<CircleIconButton> _nextButton;
	object_ptr<Ui::IconButton> _repeatButton;

	object_ptr<SpeedPillButton> _speedButton;
	object_ptr<Ui::IconButton> _volumeButton;
	object_ptr<ExpressiveSlider> _volumeSlider;
	object_ptr<QueuePillButton> _queueButton;

	object_ptr<QueuePanel> _queuePanel;

	std::shared_ptr<Data::DocumentMedia> _currentMedia;
	DocumentData *_currentDocument = nullptr;
	FullMsgId _currentContextId;
	QString _songTitle;
	Fn<void(not_null<const HistoryItem*>)> _goToItemCallback;

	QRect _coverRect;
	QRect _baseGeometry;
	crl::time _seekPositionMs = -1;
	crl::time _lastDurationMs = 0;
	bool _playing = false;
	bool _hiding = false;
	bool _hasCover = false;
	bool _extractingCover = false;
	int _coverExtractionSeq = 0;
	QImage _embeddedCover;

	CoverTheme _coverTheme;
	CoverTheme _prevCoverTheme;
	CoverTheme _targetCoverTheme;
	Ui::Animations::Simple _a_themeTransition;

	QPointer<QWidget> _previousFocus;

	base::unique_qptr<Ui::PopupMenu> _moreMenu;

	Ui::Animations::Simple _a_show;
	rpl::event_stream<> _closeRequests;
	rpl::event_stream<> _hideFinished;
	rpl::lifetime _trackLifetime;

	int _accumulatedDeltaX = 0;

};

} // namespace Rabbit::Player
