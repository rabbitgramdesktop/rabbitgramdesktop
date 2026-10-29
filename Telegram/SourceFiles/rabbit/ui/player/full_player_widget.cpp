/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/player/full_player_widget.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "media/player/media_player_instance.h"
#include "media/audio/media_audio.h"
#include "media/media_common.h"
#include "data/data_document.h"
#include "data/data_document_media.h"
#include "data/data_file_origin.h"
#include "data/data_session.h"
#include "data/data_peer.h"
#include "history/history.h"
#include "history/history_item.h"
#include "info/saved/info_saved_music_widget.h"
#include "info/info_memento.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/text/format_song_document_name.h"
#include "ui/text/format_values.h"
#include "ui/image/image.h"
#include "ui/painter.h"
#include "ui/controls/swipe_handler.h"
#include "window/window_session_controller.h"
#include "styles/style_media_player.h"
#include "styles/style_widgets.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_window.h"
#include "styles/style_info.h"
#include "styles/style_menu_icons.h"
#include "styles/style_media_view.h"

#include <QtGui/QPainterPath>
#include <QtGui/QLinearGradient>

namespace Rabbit::Player {

using Media::RepeatMode;
using Media::OrderMode;

namespace {

constexpr auto kAnimationDuration = crl::time(180);

[[nodiscard]] QString FormatTrackTime(crl::time timeMs) {
	return Ui::FormatDurationText(std::max(timeMs, crl::time(0)) / 1000);
}

} // namespace

class TopPillButton final : public Ui::RippleButton {
public:
	TopPillButton(
		QWidget *parent,
		const style::icon *icon,
		const QString &text)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _icon(icon)
	, _text(text) {
		setCursor(style::cur_pointer);
		updateNaturalSize();
	}

	void setText(const QString &text) {
		if (_text != text) {
			_text = text;
			updateNaturalSize();
			update();
		}
	}

	void setIcon(const style::icon *icon) {
		if (_icon != icon) {
			_icon = icon;
			updateNaturalSize();
			update();
		}
	}

	[[nodiscard]] int naturalWidth() const {
		return _naturalWidth;
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto r = rect();
		const auto radius = r.height() / 2.;
		const auto over = _a_over.value(isOver() ? 1. : 0.);
		const auto bg = anim::color(
			st::windowBgOver->c,
			anim::color(st::windowBgOver->c, st::windowFg->c, 0.10),
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		p.setFont(st::semiboldFont);
		p.setPen(st::windowFg);

		const auto font = st::semiboldFont;
		const auto textWidth = font->width(_text);
		const auto iconWidth = _icon ? _icon->width() : 0;
		const auto iconHeight = _icon ? _icon->height() : 0;
		const auto spacing = (_icon && !_text.isEmpty()) ? 6 : 0;
		const auto contentWidth = iconWidth + spacing + textWidth;
		const auto contentLeft = (r.width() - contentWidth) / 2;

		if (_icon) {
			_icon->paint(
				p,
				contentLeft,
				(r.height() - iconHeight) / 2,
				r.width(),
				st::windowFg->c);
		}
		if (!_text.isEmpty()) {
			p.drawText(
				QRect(contentLeft + iconWidth + spacing, 0, textWidth + 4, r.height()),
				Qt::AlignVCenter | Qt::AlignLeft,
				_text);
		}
	}

	void onStateChanged(State was, StateChangeSource source) override {
		RippleButton::onStateChanged(was, source);
		const auto over = isOver();
		if ((was & StateFlag::Over) != (state() & StateFlag::Over)) {
			_a_over.start([=] { update(); }, over ? 0. : 1., over ? 1. : 0., crl::time(150));
		}
	}

private:
	void updateNaturalSize() {
		const auto font = st::semiboldFont;
		const auto textWidth = font->width(_text);
		const auto iconWidth = _icon ? _icon->width() : 0;
		const auto spacing = (_icon && !_text.isEmpty()) ? 6 : 0;
		const auto paddingH = 14;
		_naturalWidth = paddingH * 2 + iconWidth + spacing + textWidth;
	}

	const style::icon *_icon = nullptr;
	QString _text;
	int _naturalWidth = 0;
	Ui::Animations::Simple _a_over;
};

class CircleIconButton final : public Ui::RippleButton {
public:
	CircleIconButton(QWidget *parent, const style::icon *icon)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _icon(icon) {
		setCursor(style::cur_pointer);
	}

	void setIcon(const style::icon *icon) {
		if (_icon != icon) {
			_icon = icon;
			update();
		}
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto r = rect();
		const auto radius = r.height() / 2.;
		const auto over = _a_over.value(isOver() ? 1. : 0.);
		const auto bg = anim::color(
			st::windowBgOver->c,
			anim::color(st::windowBgOver->c, st::windowFg->c, 0.10),
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		if (_icon) {
			_icon->paintInCenter(p, r, st::windowFg->c);
		}
	}

	void onStateChanged(State was, StateChangeSource source) override {
		RippleButton::onStateChanged(was, source);
		const auto over = isOver();
		if ((was & StateFlag::Over) != (state() & StateFlag::Over)) {
			_a_over.start([=] { update(); }, over ? 0. : 1., over ? 1. : 0., crl::time(150));
		}
	}

private:
	const style::icon *_icon = nullptr;
	Ui::Animations::Simple _a_over;
};

class PlayPauseButton final : public Ui::RippleButton {
public:
	PlayPauseButton(QWidget *parent, Fn<void()> geometryChangedCallback)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _geometryChanged(std::move(geometryChangedCallback)) {
		setCursor(style::cur_pointer);
	}

	void setPlaying(bool playing, anim::type animated = anim::type::normal) {
		if (_playing == playing) {
			return;
		}
		_playing = playing;
		if (animated == anim::type::instant) {
			_a_playing.stop();
			if (_geometryChanged) {
				_geometryChanged();
			}
			update();
		} else {
			_a_playing.start([=] {
				if (_geometryChanged) {
					_geometryChanged();
				}
				update();
			}, _playing ? 0. : 1., _playing ? 1. : 0., crl::time(180));
		}
	}

	[[nodiscard]] float64 playingProgress() const {
		return _a_playing.value(_playing ? 1. : 0.);
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto progress = playingProgress();
		const auto r = rect();
		const auto radius = anim::interpolate(r.height() / 2., r.height() / 3., progress);

		const auto over = _a_over.value(isOver() ? 1. : 0.);
		const auto bg = anim::color(
			st::activeButtonBg->c,
			anim::color(st::activeButtonBg->c, Qt::white, 0.15),
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		const auto cx = r.width() / 2.;
		const auto cy = r.height() / 2.;

		if (progress < 1.) {
			p.save();
			p.setOpacity(1. - progress);
			const auto s = 1. + 0.15 * progress;
			p.translate(cx, cy);
			p.scale(s, s);
			p.translate(-cx, -cy);

			QPainterPath playPath;
			const auto tw = 20.;
			const auto th = 24.;
			playPath.moveTo(cx - tw * 0.40, cy - th * 0.5);
			playPath.lineTo(cx + tw * 0.60, cy);
			playPath.lineTo(cx - tw * 0.40, cy + th * 0.5);
			playPath.closeSubpath();
			p.fillPath(playPath, st::activeButtonFg);
			p.restore();
		}

		if (progress > 0.) {
			p.save();
			p.setOpacity(progress);
			const auto s = 0.85 + 0.15 * progress;
			p.translate(cx, cy);
			p.scale(s, s);
			p.translate(-cx, -cy);

			p.setPen(Qt::NoPen);
			p.setBrush(st::activeButtonFg);
			const auto barW = 5.5;
			const auto barH = 22.;
			const auto gap = 7.;
			p.drawRoundedRect(QRectF(cx - gap / 2. - barW, cy - barH / 2., barW, barH), 2.5, 2.5);
			p.drawRoundedRect(QRectF(cx + gap / 2., cy - barH / 2., barW, barH), 2.5, 2.5);
			p.restore();
		}
	}

	void onStateChanged(State was, StateChangeSource source) override {
		RippleButton::onStateChanged(was, source);
		const auto over = isOver();
		if ((was & StateFlag::Over) != (state() & StateFlag::Over)) {
			_a_over.start([=] { update(); }, over ? 0. : 1., over ? 1. : 0., crl::time(150));
		}
	}

private:
	Fn<void()> _geometryChanged;
	bool _playing = false;
	Ui::Animations::Simple _a_playing;
	Ui::Animations::Simple _a_over;
};

FullPlayerWidget::FullPlayerWidget(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: RpWidget(parent)
, _controller(controller)
, _closeButton(this, &st::backButtonIcon, u"Назад"_q)
, _goToMessageButton(this, &st::menuIconShowInChat, u"В чат"_q)
, _headerTitle(this, st::boxTitle)
, _headerSubtitle(this, st::defaultFlatLabel)
, _trackTitle(this, st::boxTitle)
, _trackPerformer(this, st::defaultFlatLabel)
, _progressSlider(this, st::mediaPlayerPlayback)
, _timePassedLabel(this, st::mediaPlayerTime)
, _timeTotalLabel(this, st::mediaPlayerTime)
, _orderButton(this, st::mediaPlayerOrderButton)
, _prevButton(this, &st::mediaPlayerPreviousButton.icon)
, _playPauseButton(this, [=] { updatePlayPauseGeometry(); })
, _nextButton(this, &st::mediaPlayerNextButton.icon)
, _repeatButton(this, st::mediaPlayerRepeatButton)
, _volumeButton(this, st::mediaPlayerVolumeToggle)
, _volumeSlider(this, st::mediaPlayerPanelPlayback) {

	setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::StrongFocus);

	setupControls();
	setupSwipeBack();

	const auto player = Media::Player::instance();

	player->updatedNotifier(
	) | rpl::filter([=](const Media::Player::TrackState &state) {
		return (state.id.type() == AudioMsgId::Type::Song);
	}) | rpl::on_next([=](const Media::Player::TrackState &state) {
		updateTrackState(state);
	}, _trackLifetime);

	player->stops(
		AudioMsgId::Type::Song
	) | rpl::on_next([=] {
		hideAnimated();
	}, _trackLifetime);

	player->tracksFinished(
	) | rpl::on_next([=] {
		updateSongDetails();
	}, _trackLifetime);

	player->startsPlay(
		AudioMsgId::Type::Song
	) | rpl::on_next([=] {
		updateSongDetails();
	}, _trackLifetime);

	Core::App().settings().playerRepeatModeValue(
	) | rpl::on_next([=] {
		updateRepeatButton();
	}, lifetime());

	Core::App().settings().playerOrderModeValue(
	) | rpl::on_next([=] {
		updateOrderButton();
	}, lifetime());

	Core::App().settings().songVolumeChanges(
	) | rpl::on_next([=](float64 volume) {
		updateVolumeButton();
		if (!_volumeSlider->isChanging()) {
			_volumeSlider->setValue(volume);
		}
	}, lifetime());

	updateSongDetails();
	updateRepeatButton();
	updateOrderButton();
	updateVolumeButton();
	_volumeSlider->setValue(Core::App().settings().songVolume());
}

FullPlayerWidget::~FullPlayerWidget() = default;

void FullPlayerWidget::setupControls() {
	_closeButton->setClickedCallback([=] {
		hideAnimated();
	});

	_goToMessageButton->setClickedCallback([=] {
		if (_currentDocument && _currentContextId) {
			if (const auto item = _currentDocument->owner().message(_currentContextId)) {
				if (_goToItemCallback) {
					_goToItemCallback(item);
				} else {
					hideAnimated();
					if (item->isSavedMusicItem()) {
						_controller->showSection(Info::Saved::MakeMusic(item->history()->peer));
					} else {
						_controller->showMessage(item);
					}
				}
			}
		}
	});

	_headerTitle->setText(u"Сейчас играет"_q);

	_progressSlider->setChangeProgressCallback([=](float64 progress) {
		handleSeekProgress(progress);
	});
	_progressSlider->setChangeFinishedCallback([=](float64 progress) {
		handleSeekFinished(progress);
	});

	_prevButton->setClickedCallback([=] {
		Media::Player::instance()->previous(AudioMsgId::Type::Song);
	});

	_nextButton->setClickedCallback([=] {
		Media::Player::instance()->next(AudioMsgId::Type::Song);
	});

	_playPauseButton->setClickedCallback([=] {
		Media::Player::instance()->playPause(AudioMsgId::Type::Song);
	});

	_repeatButton->setClickedCallback([=] {
		auto &settings = Core::App().settings();
		settings.setPlayerRepeatMode([&] {
			switch (settings.playerRepeatMode()) {
			case RepeatMode::None: return RepeatMode::One;
			case RepeatMode::One: return RepeatMode::All;
			case RepeatMode::All: return RepeatMode::None;
			}
			return RepeatMode::None;
		}());
		Core::App().saveSettingsDelayed();
	});

	_orderButton->setClickedCallback([=] {
		auto &settings = Core::App().settings();
		settings.setPlayerOrderMode([&] {
			switch (settings.playerOrderMode()) {
			case OrderMode::Default: return OrderMode::Shuffle;
			case OrderMode::Shuffle: return OrderMode::Reverse;
			case OrderMode::Reverse: return OrderMode::Default;
			}
			return OrderMode::Default;
		}());
		Core::App().saveSettingsDelayed();
	});

	_volumeButton->setClickedCallback([=] {
		const auto current = Core::App().settings().songVolume();
		const auto target = (current > 0.)
			? 0.
			: Core::App().settings().rememberedSongVolume();
		applyVolumeChange(target);
	});

	_volumeSlider->setChangeProgressCallback([=](float64 volume) {
		applyVolumeChange(volume);
	});
	_volumeSlider->setChangeFinishedCallback([=](float64 volume) {
		if (volume > 0.) {
			Core::App().settings().setRememberedSongVolume(volume);
		}
		applyVolumeChange(volume);
		Core::App().saveSettingsDelayed();
	});
}

void FullPlayerWidget::setBaseGeometry(const QRect &geometry) {
	_baseGeometry = geometry;
	if (_a_show.animating()) {
		updateSlide();
	} else if (!_hiding && isVisible()) {
		setGeometry(_baseGeometry);
	}
}

void FullPlayerWidget::updateSlide() {
	if (_baseGeometry.isEmpty()) {
		return;
	}
	const auto progress = _a_show.value(_hiding ? 0. : 1.);
	const auto yOffset = static_cast<int>(std::round((1. - progress) * _baseGeometry.height()));
	setGeometry(
		_baseGeometry.x(),
		_baseGeometry.y() + yOffset,
		_baseGeometry.width(),
		_baseGeometry.height());
}

void FullPlayerWidget::showAnimated() {
	_hiding = false;
	show();
	raise();
	setFocus();
	_a_show.start([=] {
		updateSlide();
	}, _a_show.value(0.), 1., kAnimationDuration);
	updateSlide();
}

void FullPlayerWidget::hideAnimated() {
	if (_hiding) {
		return;
	}
	_hiding = true;
	clearFocus();
	_closeRequests.fire({});
	_a_show.start([=] {
		updateSlide();
		if (!_a_show.animating()) {
			hide();
			_hideFinished.fire({});
		}
	}, _a_show.value(1.), 0., kAnimationDuration);
	updateSlide();
}

void FullPlayerWidget::keyPressEvent(QKeyEvent *e) {
	if (e->key() == Qt::Key_Escape) {
		hideAnimated();
		e->accept();
		return;
	} else if (e->key() == Qt::Key_Space) {
		Media::Player::instance()->playPause(AudioMsgId::Type::Song);
		e->accept();
		return;
	}
	RpWidget::keyPressEvent(e);
}

void FullPlayerWidget::mousePressEvent(QMouseEvent *e) {
	if (e->button() == Qt::BackButton) {
		hideAnimated();
		e->accept();
		return;
	}
	RpWidget::mousePressEvent(e);
}

void FullPlayerWidget::wheelEvent(QWheelEvent *e) {
	const auto pixelX = e->pixelDelta().x();
	const auto angleX = e->angleDelta().x();
	const auto deltaX = pixelX ? pixelX : angleX;
	if (deltaX > 0) {
		_accumulatedDeltaX += deltaX;
		if (_accumulatedDeltaX >= 120) {
			_accumulatedDeltaX = 0;
			hideAnimated();
			e->accept();
			return;
		}
	} else if (deltaX < 0) {
		_accumulatedDeltaX = 0;
	}
	RpWidget::wheelEvent(e);
}

void FullPlayerWidget::setupSwipeBack() {
	using namespace Ui::Controls;

	auto swipeBackData = lifetime().make_state<SwipeBackResult>();

	auto update = [=](SwipeContextData data) {
		if (data.translation > 0) {
			if (!swipeBackData->callback) {
				(*swipeBackData) = SetupSwipeBack(
					this,
					[]() -> std::pair<QColor, QColor> {
						return {
							st::windowBgOver->c,
							st::windowFg->c,
						};
					});
			}
			swipeBackData->callback(data);
			return;
		} else if (swipeBackData->lifetime) {
			(*swipeBackData) = {};
		}
	};

	auto init = [=](Ui::Controls::SwipeHandlerInitData data) {
		return (data.direction == Qt::RightToLeft)
			? DefaultSwipeBackHandlerFinishData([=] {
				hideAnimated();
			})
			: SwipeHandlerFinishData();
	};

	SetupSwipeHandler({
		.widget = this,
		.scroll = v::null,
		.update = std::move(update),
		.init = std::move(init),
	});
}

void FullPlayerWidget::updateSongDetails() {
	const auto current = Media::Player::instance()->current(AudioMsgId::Type::Song);
	_currentDocument = current.audio();
	_currentContextId = current.contextId();

	if (!_currentDocument) {
		_trackTitle->setText(QString());
		_trackPerformer->setText(QString());
		_headerSubtitle->setText(QString());
		_currentMedia = nullptr;
		_hasCover = false;
		update();
		return;
	}

	const auto name = Ui::Text::FormatSongNameFor(_currentDocument);
	const auto composed = name.composedName();
	_trackTitle->setText(composed.title.isEmpty() ? _currentDocument->filename() : composed.title);
	_trackPerformer->setText(composed.performer);

	const auto item = _currentDocument->owner().message(_currentContextId);
	if (item) {
		const auto isSavedMusic = item->isSavedMusicItem();
		_headerSubtitle->setText(item->history()->peer->name());
		_goToMessageButton->setText(isSavedMusic ? u"В профиль"_q : u"В чат"_q);
		_goToMessageButton->setVisible(true);
	} else {
		_headerSubtitle->setText(QString());
		_goToMessageButton->setVisible(false);
	}

	_currentMedia = _currentDocument->createMediaView();
	if (_currentMedia) {
		if (!_currentMedia->thumbnail()) {
			const auto origin = item ? item->fullId() : Data::FileOrigin();
			_currentMedia->thumbnailWanted(origin);
		}
		_hasCover = (_currentMedia->thumbnail() != nullptr);
	}

	updateTrackState(Media::Player::instance()->getState(AudioMsgId::Type::Song));
	updateControlsGeometry();
	update();
}

void FullPlayerWidget::updateTrackState(const Media::Player::TrackState &state) {
	const auto playing = (state.state == Media::Player::State::Playing);
	if (_playing != playing) {
		_playing = playing;
		updatePlayPauseButton();
	}

	if (!_currentDocument) {
		return;
	}

	const auto frequency = state.frequency;
	_lastDurationMs = frequency ? ((state.length * 1000LL) / frequency) : 0;
	const auto positionMs = frequency ? ((state.position * 1000LL) / frequency) : 0;

	if (_seekPositionMs < 0) {
		const auto progress = (_lastDurationMs > 0)
			? (float64(positionMs) / _lastDurationMs)
			: 0.;
		_progressSlider->setValue(std::clamp(progress, 0., 1.));
		_timePassedLabel->setText(FormatTrackTime(positionMs));
		_timeTotalLabel->setText(FormatTrackTime(_lastDurationMs));
	}

	if (_currentMedia && !_hasCover && _currentMedia->thumbnail()) {
		_hasCover = true;
		update();
	}
}

void FullPlayerWidget::handleSeekProgress(float64 progress) {
	if (!_lastDurationMs) {
		return;
	}
	const auto positionMs = std::clamp(
		static_cast<crl::time>(progress * _lastDurationMs),
		crl::time(0),
		_lastDurationMs);
	if (_seekPositionMs != positionMs) {
		_seekPositionMs = positionMs;
		_timePassedLabel->setText(FormatTrackTime(_seekPositionMs));
		Media::Player::instance()->startSeeking(AudioMsgId::Type::Song);
	}
}

void FullPlayerWidget::handleSeekFinished(float64 progress) {
	if (!_lastDurationMs) {
		return;
	}
	_seekPositionMs = -1;
	Media::Player::instance()->finishSeeking(AudioMsgId::Type::Song, progress);
}

void FullPlayerWidget::applyVolumeChange(float64 volume) {
	if (volume != Core::App().settings().songVolume()) {
		Media::Player::mixer()->setSongVolume(volume);
		Core::App().settings().setSongVolume(volume);
	}
}

void FullPlayerWidget::updateRepeatButton() {
	switch (Core::App().settings().playerRepeatMode()) {
	case RepeatMode::None:
		_repeatButton->setIconOverride(
			&st::mediaPlayerRepeatDisabledIcon,
			&st::mediaPlayerRepeatDisabledIconOver);
		break;
	case RepeatMode::One:
		_repeatButton->setIconOverride(&st::mediaPlayerRepeatOneIcon);
		break;
	case RepeatMode::All:
		_repeatButton->setIconOverride(nullptr);
		break;
	}
}

void FullPlayerWidget::updateOrderButton() {
	switch (Core::App().settings().playerOrderMode()) {
	case OrderMode::Default:
		_orderButton->setIconOverride(
			&st::mediaPlayerReverseDisabledIcon,
			&st::mediaPlayerReverseDisabledIconOver);
		break;
	case OrderMode::Reverse:
		_orderButton->setIconOverride(&st::mediaPlayerReverseIcon);
		break;
	case OrderMode::Shuffle:
		_orderButton->setIconOverride(&st::mediaPlayerShuffleIcon);
		break;
	}
}

void FullPlayerWidget::updateVolumeButton() {
	const auto volume = Core::App().settings().songVolume();
	if (volume <= 0.) {
		_volumeButton->setIconOverride(&st::mediaPlayerVolumeIcon0);
	} else if (volume < 0.5) {
		_volumeButton->setIconOverride(&st::mediaPlayerVolumeIcon1);
	} else {
		_volumeButton->setIconOverride(nullptr);
	}
}

void FullPlayerWidget::updatePlayPauseButton() {
	_playPauseButton->setPlaying(_playing);
}

void FullPlayerWidget::resizeEvent(QResizeEvent *e) {
	updateControlsGeometry();
	RpWidget::resizeEvent(e);
}

void FullPlayerWidget::updateControlsGeometry() {
	const auto w = width();
	const auto h = height();
	if (w <= 0 || h <= 0) {
		return;
	}

	const auto topBarY = 14;
	const auto topBarHeight = 34;

	const auto closeW = _closeButton->naturalWidth();
	_closeButton->setGeometry(16, topBarY, closeW, topBarHeight);

	const auto goToW = _goToMessageButton->naturalWidth();
	_goToMessageButton->setGeometry(w - goToW - 16, topBarY, goToW, topBarHeight);

	const auto sideButtonsWidth = std::max(closeW, goToW) + 24;
	const auto headerTextWidth = std::max(w - 2 * sideButtonsWidth, 80);
	_headerTitle->resizeToNaturalWidth(headerTextWidth);
	_headerTitle->moveToLeft((w - _headerTitle->width()) / 2, topBarY - 2);

	_headerSubtitle->resizeToNaturalWidth(headerTextWidth);
	_headerSubtitle->moveToLeft((w - _headerSubtitle->width()) / 2, topBarY + 18);

	const auto bottomPadding = 24;
	const auto volumeY = h - bottomPadding - _volumeButton->height();
	const auto volumeSliderWidth = std::clamp(w / 3, 100, 180);
	const auto volumeTotalWidth = _volumeButton->width() + 8 + volumeSliderWidth;
	const auto volumeLeft = (w - volumeTotalWidth) / 2;

	_volumeButton->moveToLeft(volumeLeft, volumeY);
	_volumeSlider->setGeometry(
		volumeLeft + _volumeButton->width() + 8,
		volumeY + (_volumeButton->height() - st::mediaPlayerPanelPlayback.width) / 2,
		volumeSliderWidth,
		st::mediaPlayerPanelPlayback.width);

	updatePlayPauseGeometry();

	const auto controlsY = volumeY - 68;
	const auto progressY = controlsY - 48;
	const auto progressWidth = std::clamp(w - 80, 220, 480);
	const auto progressX = (w - progressWidth) / 2;
	_progressSlider->setGeometry(
		progressX,
		progressY,
		progressWidth,
		st::mediaPlayerPlayback.fullWidth);

	_timePassedLabel->moveToLeft(progressX, progressY + st::mediaPlayerPlayback.fullWidth + 4);
	_timeTotalLabel->moveToLeft(
		progressX + progressWidth - _timeTotalLabel->width(),
		progressY + st::mediaPlayerPlayback.fullWidth + 4);

	const auto textY = progressY - 56;
	const auto textWidth = std::max(w - 60, 100);
	_trackTitle->resizeToNaturalWidth(textWidth);
	_trackTitle->moveToLeft((w - _trackTitle->width()) / 2, textY);

	_trackPerformer->resizeToNaturalWidth(textWidth);
	_trackPerformer->moveToLeft((w - _trackPerformer->width()) / 2, textY + _trackTitle->height() + 4);

	const auto headerBottom = 58;
	const auto coverAvailableHeight = textY - headerBottom - 16;
	const auto maxCover = std::min({ w - 80, coverAvailableHeight, 300 });
	const auto coverSize = std::max(maxCover, 120);
	const auto coverX = (w - coverSize) / 2;
	const auto coverY = headerBottom + (coverAvailableHeight - coverSize) / 2;

	_coverRect = QRect(coverX, coverY, coverSize, coverSize);
}

void FullPlayerWidget::updatePlayPauseGeometry() {
	const auto w = width();
	const auto h = height();
	if (w <= 0 || h <= 0) {
		return;
	}

	const auto bottomPadding = 24;
	const auto volumeY = h - bottomPadding - _volumeButton->height();
	const auto controlsY = volumeY - 68;

	const auto progress = _playPauseButton ? _playPauseButton->playingProgress() : 0.;
	const auto currentPlayWidth = static_cast<int>(std::round(anim::interpolate(60, 80, progress)));

	const auto playX = (w - currentPlayWidth) / 2;
	const auto playY = controlsY;
	if (_playPauseButton) {
		_playPauseButton->setGeometry(playX, playY, currentPlayWidth, 60);
	}

	const auto btnSpacing = 20;
	const auto prevX = playX - btnSpacing - 44;
	if (_prevButton) {
		_prevButton->setGeometry(prevX, playY + (60 - 44) / 2, 44, 44);
	}

	if (_orderButton) {
		const auto orderX = prevX - btnSpacing - _orderButton->width();
		_orderButton->moveToLeft(orderX, playY + (60 - _orderButton->height()) / 2);
	}

	const auto nextX = playX + currentPlayWidth + btnSpacing;
	if (_nextButton) {
		_nextButton->setGeometry(nextX, playY + (60 - 44) / 2, 44, 44);
	}

	if (_repeatButton) {
		const auto repeatX = nextX + 44 + btnSpacing;
		_repeatButton->moveToLeft(repeatX, playY + (60 - _repeatButton->height()) / 2);
	}
}

void FullPlayerWidget::paintDefaultCoverIcon(QPainter &p, const QRect &rect) {
	p.save();
	p.setRenderHint(QPainter::Antialiasing);

	const auto cx = rect.center().x();
	const auto cy = rect.center().y();
	const auto noteW = rect.width() / 4;
	const auto noteH = rect.height() / 3;

	QPainterPath notePath;
	const auto headRadius = noteW / 3;
	notePath.addEllipse(QPointF(cx - noteW / 2, cy + noteH / 3), headRadius, headRadius * 0.75);
	notePath.addEllipse(QPointF(cx + noteW / 2, cy + noteH / 4), headRadius, headRadius * 0.75);

	p.fillPath(notePath, QColor(255, 255, 255, 220));

	QPen stemPen(QColor(255, 255, 255, 220), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	p.setPen(stemPen);
	p.drawLine(QPointF(cx - noteW / 2 + headRadius, cy + noteH / 3), QPointF(cx - noteW / 2 + headRadius, cy - noteH / 2));
	p.drawLine(QPointF(cx + noteW / 2 + headRadius, cy + noteH / 4), QPointF(cx + noteW / 2 + headRadius, cy - noteH / 2 - 4));
	p.drawLine(QPointF(cx - noteW / 2 + headRadius, cy - noteH / 2), QPointF(cx + noteW / 2 + headRadius, cy - noteH / 2 - 4));

	p.restore();
}

void FullPlayerWidget::paintEvent(QPaintEvent *e) {
	Painter p(this);

	p.fillRect(rect(), st::windowBg);
	p.fillRect(0, 0, st::lineWidth, height(), st::shadowFg);

	if (_coverRect.isValid() && _coverRect.width() > 0) {
		const auto shadowRect = _coverRect.translated(0, 6);
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(0, 0, 0, 30));
		p.drawRoundedRect(shadowRect, 18, 18);

		QPainterPath clip;
		clip.addRoundedRect(_coverRect, 18, 18);
		p.setClipPath(clip);

		auto drewThumbnail = false;
		if (_currentMedia) {
			if (const auto img = _currentMedia->thumbnail()) {
				const auto &pixmap = img->pix(_coverRect.size());
				p.drawPixmap(_coverRect, pixmap);
				drewThumbnail = true;
			}
		}

		if (!drewThumbnail) {
			QLinearGradient grad(_coverRect.topLeft(), _coverRect.bottomRight());
			grad.setColorAt(0.0, st::activeButtonBg->c);
			grad.setColorAt(1.0, anim::color(st::activeButtonBg->c, st::windowBg->c, 0.4));
			p.fillRect(_coverRect, grad);

			paintDefaultCoverIcon(p, _coverRect);
		}

		p.setClipping(false);
	}
}

} // namespace Rabbit::Player
