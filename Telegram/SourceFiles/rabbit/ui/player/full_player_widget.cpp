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
#include "media/player/media_player_dropdown.h"
#include "media/audio/media_audio.h"
#include "media/media_common.h"
#include "data/data_document.h"
#include "data/data_document_media.h"
#include "data/data_file_origin.h"
#include "data/data_file_click_handler.h"
#include "data/data_session.h"
#include "data/data_peer.h"
#include "data/data_chat.h"
#include "data/data_channel.h"
#include "data/data_forum_topic.h"
#include "data/data_saved_music.h"
#include "history/history.h"
#include "history/history_item.h"
#include "main/main_session.h"
#include "lang/lang_keys.h"
#include "info/saved/info_saved_music_widget.h"
#include "info/info_memento.h"
#include "info/info_controller.h"
#include "info/media/info_media_list_widget.h"
#include "boxes/share_box.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/ui/player/player_settings_box.h"
#include "rabbit/ui/player/cover_theme.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/scroll_area.h"
#include "ui/resize_area.h"
#include "ui/widgets/popup_menu.h"
#include "ui/widgets/menu/menu_action.h"
#include "ui/text/format_song_document_name.h"
#include "ui/text/format_values.h"
#include "ui/image/image.h"
#include "ui/painter.h"
#include "ui/controls/swipe_handler.h"
#include "window/window_session_controller.h"
#include "window/window_controller.h"
#include "window/main_window.h"
#include "window/window_peer_menu.h"
#include "styles/style_media_player.h"
#include "styles/style_widgets.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_window.h"
#include "styles/style_info.h"
#include "styles/style_menu_icons.h"
#include "styles/style_media_view.h"
#include "styles/style_chat_helpers.h"
#include "styles/style_info_profile_top_bar.h"
#include "styles/style_iv.h"
#include "styles/style_rabbit_assets.h"
#include "media/player/media_player_button.h"

#include <QtGui/QPainterPath>
#include <QtGui/QLinearGradient>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtCore/QFileInfo>

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

	void setCustomColors(QColor bg, QColor bgOver, QColor fg) {
		_customBg = bg;
		_customBgOver = bgOver;
		_customFg = fg;
		update();
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
		const auto defaultBg = st::windowBgOver->c;
		const auto defaultBgOver = anim::color(st::windowBgOver->c, st::windowFg->c, 0.10);

		const auto bg = anim::color(
			_customBg.isValid() ? _customBg : defaultBg,
			_customBgOver.isValid() ? _customBgOver : defaultBgOver,
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		p.setFont(st::semiboldFont);
		p.setPen(_customFg.isValid() ? _customFg : st::windowFg->c);

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
				_customFg.isValid() ? _customFg : st::windowFg->c);
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
		const auto paddingH = _text.isEmpty() ? ((34 - iconWidth) / 2) : 14;
		_naturalWidth = paddingH * 2 + iconWidth + spacing + textWidth;
	}

	const style::icon *_icon = nullptr;
	QString _text;
	int _naturalWidth = 0;
	QColor _customBg;
	QColor _customBgOver;
	QColor _customFg;
	Ui::Animations::Simple _a_over;
};

class CircleIconButton final : public Ui::RippleButton {
public:
	CircleIconButton(QWidget *parent, const style::icon *icon)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _icon(icon) {
		setCursor(style::cur_pointer);
	}

	void setCustomColors(QColor bg, QColor bgOver, QColor fg) {
		_customBg = bg;
		_customBgOver = bgOver;
		_customFg = fg;
		update();
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
		const auto defaultBg = st::windowBgOver->c;
		const auto defaultBgOver = anim::color(st::windowBgOver->c, st::windowFg->c, 0.10);

		const auto bg = anim::color(
			_customBg.isValid() ? _customBg : defaultBg,
			_customBgOver.isValid() ? _customBgOver : defaultBgOver,
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		if (_icon) {
			_icon->paintInCenter(p, r, _customFg.isValid() ? _customFg : st::windowFg->c);
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
	QColor _customBg;
	QColor _customBgOver;
	QColor _customFg;
	Ui::Animations::Simple _a_over;
};

class SpeedPillButton final : public Ui::RippleButton {
public:
	explicit SpeedPillButton(QWidget *parent)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _layout(st::mediaPlayerSpeedButton, [=] { update(); }, 1.) {
		setCursor(style::cur_pointer);
	}

	void setCustomColors(QColor outline, QColor bgOver, QColor iconColor) {
		_outline = outline;
		_bgOver = bgOver;
		_iconColor = iconColor;
		_layout.setIconColorOverride(iconColor.isValid() ? std::make_optional(iconColor) : std::nullopt);
		update();
	}

	void setSpeed(float64 speed) {
		const auto rounded = base::SafeRound(speed * 10.) / 10.;
		if (!Media::EqualSpeeds(_speed, rounded)) {
			_speed = rounded;
			_layout.setSpeed(rounded);
			update();
		}
	}

	void setContextMenuCallback(Fn<void(QPoint)> callback) {
		_contextMenuCallback = std::move(callback);
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto r = rect();
		const auto radius = r.height() / 2.;
		const auto over = _a_over.value(isOver() ? 1. : 0.);

		const auto defaultBgOver = anim::color(
			st::windowBgOver->c,
			anim::color(st::windowBgOver->c, st::windowFg->c, 0.10),
			over);
		const auto hoverBg = _bgOver.isValid()
			? anim::color(QColor(0, 0, 0, 0), _bgOver, over)
			: anim::color(QColor(0, 0, 0, 0), defaultBgOver, over);

		const auto penColor = _outline.isValid()
			? _outline
			: anim::color(st::windowSubTextFg->c, st::windowFg->c, over);

		p.setPen(QPen(penColor, 1.5));
		p.setBrush(hoverBg);
		p.drawRoundedRect(QRectF(r).adjusted(0.75, 0.75, -0.75, -0.75), radius, radius);

		p.save();
		p.translate(
			(r.width() - st::mediaPlayerSpeedButton.size.width()) / 2,
			(r.height() - st::mediaPlayerSpeedButton.size.height()) / 2);
		_layout.paint(p, isOver(), !Media::EqualSpeeds(_speed, 1.));
		p.restore();
	}

	void mousePressEvent(QMouseEvent *e) override {
		if (e->button() == Qt::RightButton) {
			if (_contextMenuCallback) {
				_contextMenuCallback(e->globalPosition().toPoint());
			}
			e->accept();
			return;
		}
		RippleButton::mousePressEvent(e);
	}

	void onStateChanged(State was, StateChangeSource source) override {
		RippleButton::onStateChanged(was, source);
		const auto over = isOver();
		if ((was & StateFlag::Over) != (state() & StateFlag::Over)) {
			_a_over.start([=] { update(); }, over ? 0. : 1., over ? 1. : 0., crl::time(150));
		}
	}

private:
	Media::Player::SpeedButtonLayout _layout;
	float64 _speed = 1.0;
	QColor _outline;
	QColor _bgOver;
	QColor _iconColor;
	Fn<void(QPoint)> _contextMenuCallback;
	Ui::Animations::Simple _a_over;
};

class QueuePillButton final : public Ui::RippleButton {
public:
	QueuePillButton(QWidget *parent, const style::icon *icon)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _icon(icon) {
		setCursor(style::cur_pointer);
	}

	void setCustomColors(QColor bg, QColor bgOver, QColor iconColor, QColor accent) {
		_bg = bg;
		_bgOver = bgOver;
		_iconColor = iconColor;
		_accent = accent;
		update();
	}

	void setActive(bool active) {
		if (_active != active) {
			_active = active;
			_a_active.start([=] { update(); }, _active ? 0. : 1., _active ? 1. : 0., crl::time(150));
		}
	}

	[[nodiscard]] bool isActive() const {
		return _active;
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto r = rect();
		const auto radius = r.height() / 2.;
		const auto over = _a_over.value(isOver() ? 1. : 0.);
		const auto activeProgress = _a_active.value(_active ? 1. : 0.);

		const auto defaultBg = st::windowBgOver->c;
		const auto defaultBgOver = anim::color(st::windowBgOver->c, st::windowFg->c, 0.10);

		const auto normalBg = anim::color(
			_bg.isValid() ? _bg : defaultBg,
			_bgOver.isValid() ? _bgOver : defaultBgOver,
			over);
		const auto accentBg = _accent.isValid()
			? _accent
			: st::activeButtonBg->c;
		const auto accentBgOver = anim::color(accentBg, Qt::white, 0.15);
		const auto activeBg = anim::color(accentBg, accentBgOver, over);

		const auto fillBg = anim::color(normalBg, activeBg, activeProgress);

		p.setPen(Qt::NoPen);
		p.setBrush(fillBg);
		p.drawRoundedRect(r, radius, radius);

		if (_icon) {
			const auto normalFg = _iconColor.isValid() ? _iconColor : st::windowFg->c;
			const auto activeFg = Qt::white;
			const auto fg = anim::color(normalFg, activeFg, activeProgress);
			_icon->paintInCenter(p, r, fg);
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
	bool _active = false;
	QColor _bg;
	QColor _bgOver;
	QColor _iconColor;
	QColor _accent;
	Ui::Animations::Simple _a_over;
	Ui::Animations::Simple _a_active;
};

class PlayPauseButton final : public Ui::RippleButton {
public:
	PlayPauseButton(QWidget *parent, Fn<void()> geometryChangedCallback)
	: RippleButton(parent, st::defaultRippleAnimation)
	, _geometryChanged(std::move(geometryChangedCallback)) {
		setCursor(style::cur_pointer);
	}

	void setCustomColors(QColor bg, QColor bgOver, QColor fg) {
		_customBg = bg;
		_customBgOver = bgOver;
		_customFg = fg;
		update();
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
		const auto defaultBg = st::activeButtonBg->c;
		const auto defaultBgOver = anim::color(st::activeButtonBg->c, Qt::white, 0.15);

		const auto bg = anim::color(
			_customBg.isValid() ? _customBg : defaultBg,
			_customBgOver.isValid() ? _customBgOver : defaultBgOver,
			over);

		p.setPen(Qt::NoPen);
		p.setBrush(bg);
		p.drawRoundedRect(r, radius, radius);

		const auto cx = r.width() / 2.;
		const auto cy = r.height() / 2.;
		const auto fg = _customFg.isValid() ? _customFg : st::activeButtonFg->c;

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
			p.fillPath(playPath, fg);
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
			p.setBrush(fg);
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
	QColor _customBg;
	QColor _customBgOver;
	QColor _customFg;
	Ui::Animations::Simple _a_playing;
	Ui::Animations::Simple _a_over;
};

class ExpressiveSlider final : public Ui::RpWidget {
public:
	template <typename Style>
	ExpressiveSlider(QWidget *parent, const Style &st)
	: RpWidget(parent) {
		setCursor(style::cur_pointer);
		setMouseTracking(true);
	}

	void setValue(float64 value) {
		value = std::clamp(value, 0.0, 1.0);
		if (_value != value) {
			_value = value;
			update();
		}
	}

	[[nodiscard]] float64 value() const {
		return _value;
	}

	void setCustomColors(
			std::optional<QColor> active,
			std::optional<QColor> inactive,
			std::optional<QColor> thumb) {
		_customActive = active;
		_customInactive = inactive;
		_customThumb = thumb;
		update();
	}

	void setChangeProgressCallback(Fn<void(float64)> callback) {
		_changeProgressCallback = std::move(callback);
	}

	void setChangeFinishedCallback(Fn<void(float64)> callback) {
		_changeFinishedCallback = std::move(callback);
	}

	[[nodiscard]] bool isChanging() const {
		return _isDragging;
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		const auto r = rect();
		const auto overFactor = _a_hover.value((_isOver || _isDragging) ? 1. : 0.);

		const auto restingHeight = 6.0;
		const auto expandedHeight = 10.0;
		const auto trackHeight = anim::interpolate(restingHeight, expandedHeight, overFactor);
		const auto trackRadius = trackHeight / 2.0;
		const auto trackY = (r.height() - trackHeight) / 2.0;

		const auto activeColor = _customActive.value_or(st::mediaPlayerPlayback.activeFg->c);
		const auto inactiveColor = _customInactive.value_or(anim::color(st::windowSubTextFg->c, Qt::transparent, 0.5));
		const auto thumbColor = _customThumb.value_or(activeColor);

		const auto totalWidth = static_cast<float64>(r.width());
		const auto activeWidth = std::clamp(_value * totalWidth, 0.0, totalWidth);

		const auto restingThumbH = 14.0;
		const auto expandedThumbH = 18.0;
		const auto thumbH = anim::interpolate(restingThumbH, expandedThumbH, overFactor);
		const auto thumbW = 4.0;
		const auto thumbX = std::clamp(activeWidth, thumbW / 2.0, totalWidth - thumbW / 2.0);
		const auto thumbY = (r.height() - thumbH) / 2.0;

		const auto gap = 3.0;

		// Inactive track (right of thumb)
		const auto inactiveStart = std::min(thumbX + thumbW / 2.0 + gap, totalWidth);
		const auto inactiveWidth = totalWidth - inactiveStart;
		if (inactiveWidth > 0.5) {
			p.setPen(Qt::NoPen);
			p.setBrush(inactiveColor);
			p.drawRoundedRect(
				QRectF(inactiveStart, trackY, inactiveWidth, trackHeight),
				trackRadius,
				trackRadius);
		}

		// Active track (left of thumb)
		const auto activeEnd = std::max(thumbX - thumbW / 2.0 - gap, 0.0);
		if (activeEnd > 0.5) {
			p.setPen(Qt::NoPen);
			p.setBrush(activeColor);
			p.drawRoundedRect(
				QRectF(0.0, trackY, activeEnd, trackHeight),
				trackRadius,
				trackRadius);
		}

		// Thumb capsule marker
		p.setPen(Qt::NoPen);
		p.setBrush(thumbColor);
		p.drawRoundedRect(
			QRectF(thumbX - thumbW / 2.0, thumbY, thumbW, thumbH),
			thumbW / 2.0,
			thumbW / 2.0);
	}

	void mousePressEvent(QMouseEvent *e) override {
		if (e->button() == Qt::LeftButton) {
			_isDragging = true;
			updateValueFromPos(e->pos().x());
			if (_changeProgressCallback) {
				_changeProgressCallback(_value);
			}
			updateHoverAnimation();
		}
	}

	void mouseMoveEvent(QMouseEvent *e) override {
		if (_isDragging) {
			updateValueFromPos(e->pos().x());
			if (_changeProgressCallback) {
				_changeProgressCallback(_value);
			}
		}
	}

	void mouseReleaseEvent(QMouseEvent *e) override {
		if (e->button() == Qt::LeftButton && _isDragging) {
			_isDragging = false;
			updateValueFromPos(e->pos().x());
			if (_changeFinishedCallback) {
				_changeFinishedCallback(_value);
			}
			updateHoverAnimation();
		}
	}

	void enterEventHook(QEnterEvent *e) override {
		_isOver = true;
		updateHoverAnimation();
	}

	void leaveEventHook(QEvent *e) override {
		_isOver = false;
		updateHoverAnimation();
	}

private:
	void updateValueFromPos(int posX) {
		const auto w = width();
		if (w > 0) {
			setValue(std::clamp(static_cast<float64>(posX) / w, 0.0, 1.0));
		}
	}

	void updateHoverAnimation() {
		const auto over = (_isOver || _isDragging);
		if (_hoverTarget == over) {
			return;
		}
		_hoverTarget = over;
		const auto from = _a_hover.value(_hoverTarget ? 0. : 1.);
		const auto to = _hoverTarget ? 1. : 0.;
		_a_hover.start([=] { update(); }, from, to, crl::time(150));
	}

	float64 _value = 0.0;
	bool _isDragging = false;
	bool _isOver = false;
	bool _hoverTarget = false;
	std::optional<QColor> _customActive;
	std::optional<QColor> _customInactive;
	std::optional<QColor> _customThumb;
	Ui::Animations::Simple _a_hover;
	Fn<void(float64)> _changeProgressCallback;
	Fn<void(float64)> _changeFinishedCallback;
};

class QueuePanel final : public Ui::RpWidget, private Info::AbstractController {
public:
	QueuePanel(
		QWidget *parent,
		not_null<Window::SessionController*> controller)
	: RpWidget(parent)
	, Info::AbstractController(controller)
	, _controller(controller)
	, _scroll(base::make_unique_q<Ui::ScrollArea>(this, st::mediaPlayerScroll))
	, _closeButton(this, st::boxTitleClose)
	, _resizeArea(this)
	, _listBg(st::windowBg->c) {

		_resizeArea->setGeometry(0, 0, 8, height());
		_resizeArea->addMoveLeftCallback([=](int globalLeft) {
			if (!parentWidget() || _isFullscreen) return;
			const auto parentW = parentWidget()->width();
			if (parentW < 700) return;
			const auto right = parentWidget()->mapToGlobal(QPoint(parentW, 0)).x();
			const auto newWidth = right - globalLeft;
			const auto minW = 260;
			const auto maxW = std::clamp(parentW - 360, minW, 600);
			const auto clamped = std::clamp(newWidth, minW, maxW);
			if (_width != clamped) {
				_width = clamped;
				if (_widthChangedCallback) {
					_widthChangedCallback(_width);
				}
			}
		});

		_closeButton->setClickedCallback([=] {
			hide();
			if (_closeCallback) {
				_closeCallback();
			}
		});

		Media::Player::instance()->playlistChanges(
			AudioMsgId::Type::Song
		) | rpl::on_next([=] {
			refreshList();
		}, lifetime());

		ensureCreated();
	}

	void setFullscreen(bool fullscreen) {
		if (_isFullscreen != fullscreen) {
			_isFullscreen = fullscreen;
			_resizeArea->setVisible(!fullscreen);
			update();
		}
	}

	[[nodiscard]] bool isFullscreen() const {
		return _isFullscreen;
	}

	void setCustomColors(const CoverTheme &theme) {
		_coverTheme = theme;
		const auto bg = _coverTheme.valid
			? anim::color(_coverTheme.backgroundTop, Qt::black, 0.25)
			: st::windowBg->c;
		_listBg.update(bg);
		if (theme.valid) {
			_closeButton->setIconColorOverride(theme.text);
		} else {
			_closeButton->setIconColorOverride(std::nullopt);
		}
		if (const auto widget = _scroll->widget()) {
			widget->update();
		}
		update();
	}

	void setWidthChangedCallback(Fn<void(int)> callback) {
		_widthChangedCallback = std::move(callback);
	}

	void setCloseCallback(Fn<void()> callback) {
		_closeCallback = std::move(callback);
	}

	[[nodiscard]] int currentWidth() const {
		return _width;
	}

	void refresh() {
		refreshList();
	}

	void scrollPlaylistToCurrentTrack() {
		if (const auto list = static_cast<Info::Media::ListWidget*>(_scroll->widget())) {
			const auto rect = list->getCurrentSongGeometry();
			if (rect.isValid() && !rect.isEmpty()) {
				_scroll->scrollToY(rect.y() - st::infoMediaMargin.top());
			}
		}
	}

protected:
	void keyPressEvent(QKeyEvent *e) override {
		if (e->key() == Qt::Key_Escape) {
			hide();
			if (_closeCallback) {
				_closeCallback();
			}
			e->accept();
			return;
		}
		RpWidget::keyPressEvent(e);
	}

	void resizeEvent(QResizeEvent *e) override {
		RpWidget::resizeEvent(e);
		_resizeArea->setGeometry(0, 0, 8, height());
		if (_isFullscreen) {
			_resizeArea->hide();
		} else {
			_resizeArea->show();
			_resizeArea->raise();
		}
		const auto headerH = 48;
		_closeButton->moveToRight(10, (headerH - _closeButton->height()) / 2);
		const auto leftSkip = _isFullscreen ? 0 : 1;
		_scroll->setGeometry(
			leftSkip,
			headerH,
			std::max(width() - leftSkip, 0),
			std::max(height() - headerH, 0));
		if (const auto widget = static_cast<RpWidget*>(_scroll->widget())) {
			widget->resizeToWidth(_scroll->width());
		}
	}

	void paintEvent(QPaintEvent *e) override {
		Painter p(this);
		const auto r = rect();

		const auto bg = _coverTheme.valid
			? anim::color(_coverTheme.backgroundTop, Qt::black, 0.25)
			: st::windowBg->c;
		p.fillRect(r, bg);

		const auto border = _coverTheme.valid
			? _coverTheme.outline
			: st::shadowFg->c;
		if (!_isFullscreen) {
			p.fillRect(0, 0, 1, height(), border);
		}
		p.fillRect(0, 47, width(), 1, border);

		p.setFont(st::semiboldFont);
		p.setPen(_coverTheme.valid ? _coverTheme.text : st::windowFg->c);
		p.drawText(
			QRect(18, 0, width() - 56, 48),
			Qt::AlignVCenter | Qt::AlignLeft,
			u"Очередь воспроизведения"_q);
	}

private:
	Info::Key key() const override {
		if (_listMusicPeer) {
			return Info::Key(Info::Saved::MusicTag{ _listMusicPeer });
		} else if (const auto topic = listTopic()) {
			return Info::Key(topic);
		} else if (const auto sublist = listSublist()) {
			return Info::Key(sublist);
		}
		return Info::Key(_listPeer);
	}

	PeerData *migrated() const override {
		return _listMigratedPeer;
	}

	Info::Section section() const override {
		return _listMusicPeer
			? Info::Section(Info::Section::Type::SavedMusic)
			: Info::Section(Info::Section::MediaType::MusicFile);
	}

	style::color listBackground() const override {
		const auto bg = _coverTheme.valid
			? anim::color(_coverTheme.backgroundTop, Qt::black, 0.25)
			: st::windowBg->c;
		_listBg.update(bg);
		return _listBg.color();
	}

	[[nodiscard]] Data::ForumTopic *listTopic() const {
		return (_listPeer && _listTopicRootId)
			? _listPeer->forumTopicFor(_listTopicRootId)
			: nullptr;
	}

	[[nodiscard]] Data::SavedSublist *listSublist() const {
		return (_listPeer && _listSublistPeerId)
			? _listPeer->monoforumSublistFor(_listSublistPeerId)
			: nullptr;
	}

	void ensureCreated() {
		refreshList();
	}

	void refreshList() {
		const auto player = Media::Player::instance();
		const auto current = player->current(AudioMsgId::Type::Song);
		const auto contextId = current.contextId();
		const auto context = player->playlistContext(AudioMsgId::Type::Song);
		auto savedMusicItem = false;
		const auto peer = [&]() -> PeerData* {
			if (const auto document = current.audio()) {
				if (&document->session() != &_controller->session()) {
					return nullptr;
				}
			}
			const auto item = contextId
				? _controller->session().data().message(contextId)
				: nullptr;
			const auto media = item ? item->media() : nullptr;
			const auto document = media ? media->document() : nullptr;
			if (!document
				|| !document->isSharedMediaMusic()
				|| !Media::Player::IsRealPlaybackContext(item)) {
				return nullptr;
			}
			savedMusicItem = item->isSavedMusicItem();
			const auto result = item->history()->peer;
			if (const auto migrated = result->migrateTo()) {
				return migrated;
			}
			return result;
		}();
		const auto migrated = peer ? peer->migrateFrom() : nullptr;
		const auto listPeer = savedMusicItem ? nullptr : peer;
		const auto listMusicPeer = savedMusicItem ? peer : nullptr;
		const auto listTopicRootId = savedMusicItem
			? MsgId()
			: context.topicRootId;
		const auto listSublistPeerId = savedMusicItem
			? PeerId()
			: context.monoforumPeerId;
		const auto scoped = listTopicRootId || listSublistPeerId;
		const auto listMigratedPeer = (savedMusicItem || scoped)
			? nullptr
			: migrated;

		if (_listPeer != listPeer
			|| _listMusicPeer != listMusicPeer
			|| _listMigratedPeer != listMigratedPeer
			|| _listTopicRootId != listTopicRootId
			|| _listSublistPeerId != listSublistPeerId) {
			_scroll->takeWidget<QWidget>().destroy();
			_listPeer = _listMusicPeer = _listMigratedPeer = nullptr;
			_listTopicRootId = MsgId();
			_listSublistPeerId = PeerId();
		}
		if ((listPeer && !_listPeer) || (listMusicPeer && !_listMusicPeer)) {
			_listPeer = listPeer;
			_listMusicPeer = listMusicPeer;
			_listMigratedPeer = listMigratedPeer;
			_listTopicRootId = listTopicRootId;
			_listSublistPeerId = listSublistPeerId;
			auto list = object_ptr<Info::Media::ListWidget>(this, static_cast<Info::AbstractController*>(this));
			if (listMusicPeer && listMusicPeer->isSelf()) {
				list->setReorderDescriptor({
					.save = [=](
							int oldPosition,
							int newPosition,
							Fn<void()> done,
							Fn<void()> fail) {
						_controller->session().data().savedMusic().reorder(
							oldPosition,
							newPosition);
						done();
					}
				});
			}

			const auto weak = _scroll->setOwnedWidget(std::move(list));

			const auto w = (_scroll->width() > 0)
				? _scroll->width()
				: (width() > 0)
				? width()
				: _width;
			weak->resizeToWidth(w);

			weak->scrollToRequests(
			) | rpl::on_next([this](int newScrollTop) {
				_scroll->scrollToY(newScrollTop);
			}, weak->lifetime());

			using namespace rpl::mappers;
			rpl::combine(
				_scroll->scrollTopValue(),
				_scroll->heightValue(),
				tuple(_1, _1 + _2)
			) | rpl::on_next([=](int top, int bottom) {
				weak->setVisibleTopBottom(top, bottom);
			}, weak->lifetime());

			const auto type = listMusicPeer
				? Storage::SharedMediaType::MusicFile
				: section().mediaType();
			const auto topic = listTopic();
			const auto sublist = listSublist();
			auto musicMemento = Info::Saved::MusicMemento(peer);
			auto mediaMemento = topic
				? std::make_unique<Info::Media::Memento>(topic, type)
				: sublist
				? std::make_unique<Info::Media::Memento>(sublist, type)
				: std::make_unique<Info::Media::Memento>(
					peer,
					_listMigratedPeer ? _listMigratedPeer->id : PeerId(),
					type);
			auto &memento = listMusicPeer
				? musicMemento.media()
				: *mediaMemento;
			memento.setAroundId(contextId);
			memento.setIdsLimit(32);
			memento.setScrollTopItem({ contextId, peer->session().uniqueId() });
			memento.setScrollTopShift(-st::infoMediaMargin.top());
			weak->restoreState(&memento);
		}
	}

	const not_null<Window::SessionController*> _controller;
	const base::unique_qptr<Ui::ScrollArea> _scroll;
	const object_ptr<Ui::IconButton> _closeButton;
	object_ptr<Ui::ResizeArea> _resizeArea;

	PeerData *_listPeer = nullptr;
	PeerData *_listMusicPeer = nullptr;
	PeerData *_listMigratedPeer = nullptr;
	MsgId _listTopicRootId;
	PeerId _listSublistPeerId;

	int _width = 320;
	bool _isFullscreen = false;
	CoverTheme _coverTheme;
	mutable style::internal::OwnedColor _listBg;
	Fn<void(int)> _widthChangedCallback;
	Fn<void()> _closeCallback;
};

FullPlayerWidget::FullPlayerWidget(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: RpWidget(parent)
, _controller(controller)
, _closeButton(this, &st::backButtonIcon, u"Назад"_q)
, _moreButton(this, &st::infoProfileTopBarActionMore)
, _headerTitle(this, st::boxTitle)
, _headerSubtitle(this, st::defaultFlatLabel)
, _trackTitle(this, st::boxTitle)
, _trackPerformer(this, st::defaultFlatLabel)
, _progressSlider(this, st::mediaPlayerPlayback)
, _timePassedLabel(this, st::defaultFlatLabel)
, _timeTotalLabel(this, st::defaultFlatLabel)
, _orderButton(this, st::mediaPlayerOrderButton)
, _prevButton(this, &st::mediaPlayerPreviousButton.icon)
, _playPauseButton(this, [=] { updatePlayPauseGeometry(); })
, _nextButton(this, &st::mediaPlayerNextButton.icon)
, _repeatButton(this, st::mediaPlayerRepeatButton)
, _speedButton(this)
, _volumeButton(this, st::mediaPlayerVolumeToggle)
, _volumeSlider(this, st::mediaPlayerPanelPlayback)
, _queueButton(this, &st::menuIconReorder)
, _queuePanel(this, _controller) {

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

	Core::App().settings().audioPlaybackSpeedChanges(
	) | rpl::on_next([=](const Core::Settings::PlaybackSpeed &speed) {
		_speedButton->setSpeed(speed.enabled ? speed.value : 1.0);
	}, lifetime());

	RabbitSettings::JsonSettings::Events(u"player_cover_colors_fullscreen"_q
	) | rpl::on_next([=] {
		updateCoverTheme();
	}, lifetime());

	updateSongDetails();
	updateRepeatButton();
	updateOrderButton();
	updateVolumeButton();
	_volumeSlider->setValue(Core::App().settings().songVolume());
	_speedButton->setSpeed(Core::App().settings().audioPlaybackSpeed());
}

FullPlayerWidget::~FullPlayerWidget() = default;

void FullPlayerWidget::setupControls() {
	_closeButton->setClickedCallback([=] {
		hideAnimated();
	});

	_moreButton->setClickedCallback([=] {
		showMoreMenu();
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

	_speedButton->setClickedCallback([=] {
		const auto cur = Core::App().settings().audioPlaybackSpeed();
		float64 next = 1.0;
		if (std::abs(cur - 1.0) < 0.1) {
			next = 1.5;
		} else if (std::abs(cur - 1.5) < 0.1) {
			next = 2.0;
		} else {
			next = 1.0;
		}
		Media::Player::instance()->setPlaybackSpeed(next);
		Core::App().settings().setAudioPlaybackSpeed(next);
		Core::App().saveSettingsDelayed();
		_speedButton->setSpeed(next);
	});

	_speedButton->setContextMenuCallback([=](QPoint globalPos) {
		auto menu = base::make_unique_q<Ui::PopupMenu>(this, st::popupMenuWithIcons);
		const auto cur = Core::App().settings().audioPlaybackSpeed();
		Media::Player::FillSpeedMenu(
			menu->menu(),
			st::mediaPlayerSpeedMenu,
			rpl::single(cur),
			[=](float64 speed) {
				Media::Player::instance()->setPlaybackSpeed(speed);
				Core::App().settings().setAudioPlaybackSpeed(speed);
				Core::App().saveSettingsDelayed();
				_speedButton->setSpeed(speed);
			});
		_moreMenu = std::move(menu);
		_moreMenu->popup(globalPos);
	});

	_queuePanel->hide();
	_queuePanel->setWidthChangedCallback([=](int) {
		updateControlsGeometry();
		update();
	});
	_queuePanel->setCloseCallback([=] {
		_queueButton->setActive(false);
		updateControlsGeometry();
		update();
	});

	_queueButton->setClickedCallback([=] {
		toggleQueue();
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

void FullPlayerWidget::toggleQueue() {
	if (_queuePanel->isHidden()) {
		_queuePanel->refresh();
		_queuePanel->show();
		_queuePanel->raise();
		_queuePanel->scrollPlaylistToCurrentTrack();
		_queueButton->setActive(true);
	} else {
		_queuePanel->hide();
		_queueButton->setActive(false);
	}
	updateControlsGeometry();
	if (_queuePanel && !_queuePanel->isHidden()) {
		_queuePanel->scrollPlaylistToCurrentTrack();
	}
	update();
}

void FullPlayerWidget::showMoreMenu() {
	if (!_currentDocument) {
		return;
	}
	_moreMenu = base::make_unique_q<Ui::PopupMenu>(this, st::popupMenuWithIcons);
	const auto item = _currentDocument->owner().message(_currentContextId);
	const auto origin = item ? item->fullId() : Data::FileOrigin();
	const auto session = &_controller->session();
	const auto show = _controller->uiShow();
	const auto isFromUserProfile = item
		&& item->isSavedMusicItem()
		&& item->history()->peer->isSelf();

	// 1) "сохранить в..." подменю: (профиль, избранное, сохранить как)
	auto saveSubmenu = std::make_unique<Ui::PopupMenu>(this, st::popupMenuWithIcons);
	const auto rawSaveSubmenu = saveSubmenu.get();

	rawSaveSubmenu->addAction(
		u"Профиль"_q,
		[=] {
			session->data().savedMusic().save(_currentDocument, origin);
			show->showToast(tr::lng_saved_music_added(tr::now));
		},
		&st::menuIconProfile);

	rawSaveSubmenu->addAction(
		u"Избранное"_q,
		[=] {
			if (item) {
				Window::ForwardToSelf(show, Data::ForwardDraft{ .ids = { 1, item->fullId() } });
			} else {
				show->showToast(u"Не удалось найти сообщение"_q);
			}
		},
		&st::menuIconSavedMessages);

	const auto saveAsHandler = [=] {
		DocumentSaveClickHandler::SaveAndTrack(
			_currentContextId,
			_currentDocument,
			DocumentSaveClickHandler::Mode::ToNewFile);
	};

	rawSaveSubmenu->addAction(
		u"Сохранить как"_q,
		saveAsHandler,
		&st::menuIconDownload);

	_moreMenu->addAction(
		u"Сохранить в..."_q,
		std::move(saveSubmenu),
		&st::menuIconDownload);

	// 2) разделитель
	_moreMenu->addSeparator();

	// 3) переслать
	_moreMenu->addAction(
		u"Переслать"_q,
		[=] {
			if (item) {
				FastShareMessage(_controller, item);
			}
		},
		&st::menuIconForward);

	// 4) кнопка удалить из профиля красную если трек играет из профиля юзера
	if (isFromUserProfile) {
		_moreMenu->addAction(
			base::make_unique_q<Ui::Menu::Action>(
				_moreMenu->menu(),
				st::menuWithIconsAttention,
				Ui::Menu::CreateAction(
					_moreMenu->menu().get(),
					u"Удалить из профиля"_q,
					[=] {
						session->data().savedMusic().remove(_currentDocument);
						show->showToast(u"Удалено из профиля"_q);
					}),
				&st::menuIconDeleteAttention,
				&st::menuIconDeleteAttention));
	}

	// 6) кнопка детали в которой будет подменю: (Формат, Битрейт, размер файла, длительность, имя файла)
	auto detailsSubmenu = std::make_unique<Ui::PopupMenu>(this, st::popupMenuWithIcons);
	const auto rawDetailsSubmenu = detailsSubmenu.get();

	const auto format = [&] {
		const auto ext = QFileInfo(_currentDocument->filename()).suffix().toUpper();
		if (!ext.isEmpty()) {
			return ext;
		}
		const auto mime = _currentDocument->mimeString();
		if (mime.contains("mp3") || mime.contains("mpeg")) return u"MP3"_q;
		if (mime.contains("flac")) return u"FLAC"_q;
		if (mime.contains("ogg")) return u"OGG"_q;
		if (mime.contains("wav")) return u"WAV"_q;
		if (mime.contains("m4a") || mime.contains("mp4")) return u"M4A"_q;
		return mime.isEmpty() ? u"Audio"_q : mime;
	}();

	const auto durationMs = _lastDurationMs > 0 ? _lastDurationMs : _currentDocument->duration();
	const auto bitrateStr = [&] {
		if (durationMs > 0 && _currentDocument->size > 0) {
			const auto kbps = int(std::round((_currentDocument->size * 8.0) / (durationMs)));
			return u"%1 кбит/с"_q.arg(kbps);
		}
		return u"—"_q;
	}();

	const auto sizeStr = Ui::FormatSizeText(_currentDocument->size);
	const auto durationStr = FormatTrackTime(durationMs);
	const auto filenameStr = _currentDocument->filename().isEmpty()
		? _songTitle
		: _currentDocument->filename();

	const auto addDetailItem = [&](const QString &label, const QString &val) {
		rawDetailsSubmenu->addAction(
			u"%1: %2"_q.arg(label, val),
			[=] {
				QGuiApplication::clipboard()->setText(val);
				show->showToast(u"Скопировано в буфер обмена"_q);
			});
	};

	addDetailItem(u"Формат"_q, format);
	addDetailItem(u"Битрейт"_q, bitrateStr);
	addDetailItem(u"Размер файла"_q, sizeStr);
	addDetailItem(u"Длительность"_q, durationStr);
	addDetailItem(u"Имя файла"_q, filenameStr);

	_moreMenu->addAction(
		u"Детали"_q,
		std::move(detailsSubmenu),
		&st::menuIconInfo);

	// 7) и кнопка настройки плеера
	_moreMenu->addAction(
		u"Настройки плеера"_q,
		[=] {
			Rabbit::ShowPlayerSettingsBox(_controller);
		},
		&st::menuIconSettings);

	_moreMenu->popup(_moreButton->mapToGlobal(QPoint(_moreButton->width(), _moreButton->height() + 4)));
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
	_previousFocus = _controller->widget()->focusWidget();
	if (_previousFocus) {
		_previousFocus->clearFocus();
	}
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
	if (_previousFocus) {
		_previousFocus->setFocus();
		_previousFocus = nullptr;
	}
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
		if (_queuePanel && !_queuePanel->isHidden()) {
			toggleQueue();
			e->accept();
			return;
		}
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
		if (_queuePanel && !_queuePanel->isHidden()) {
			toggleQueue();
			e->accept();
			return;
		}
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
				if (_queuePanel && !_queuePanel->isHidden()) {
					toggleQueue();
				} else {
					hideAnimated();
				}
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

void FullPlayerWidget::applyCoverTheme(const CoverTheme &theme) {
	_coverTheme = theme;
	if (theme.valid) {
		_trackTitle->setTextColorOverride(theme.text);
		_trackPerformer->setTextColorOverride(theme.textSecondary);
		_headerTitle->setTextColorOverride(theme.text);
		_headerSubtitle->setTextColorOverride(theme.textSecondary);
		_timePassedLabel->setTextColorOverride(theme.textSecondary);
		_timeTotalLabel->setTextColorOverride(theme.textSecondary);

		_closeButton->setCustomColors(theme.buttonBg, theme.buttonBgOver, theme.text);
		_moreButton->setCustomColors(theme.buttonBg, theme.buttonBgOver, theme.text);
		_speedButton->setCustomColors(theme.outline, theme.buttonBgOver, theme.text);
		_queueButton->setCustomColors(theme.buttonBg, theme.buttonBgOver, theme.text, theme.accent);
		_prevButton->setCustomColors(theme.buttonBg, theme.buttonBgOver, theme.accentSecondary);
		_nextButton->setCustomColors(theme.buttonBg, theme.buttonBgOver, theme.accentSecondary);
		_playPauseButton->setCustomColors(
			theme.accent,
			anim::color(theme.accent, Qt::white, 0.15),
			theme.playPauseFg.isValid() ? theme.playPauseFg : theme.text);

		_progressSlider->setCustomColors(theme.accent, theme.buttonBg, theme.accent);
		_volumeSlider->setCustomColors(theme.accent, theme.buttonBg, theme.accent);

		_volumeButton->setIconColorOverride(theme.text);

		updateRepeatButton();
		updateOrderButton();

		if (_queuePanel) {
			_queuePanel->setCustomColors(theme);
		}
	} else {
		_trackTitle->setTextColorOverride(std::nullopt);
		_trackPerformer->setTextColorOverride(std::nullopt);
		_headerTitle->setTextColorOverride(std::nullopt);
		_headerSubtitle->setTextColorOverride(std::nullopt);
		_timePassedLabel->setTextColorOverride(std::nullopt);
		_timeTotalLabel->setTextColorOverride(std::nullopt);

		_closeButton->setCustomColors(QColor(), QColor(), QColor());
		_moreButton->setCustomColors(QColor(), QColor(), QColor());
		_speedButton->setCustomColors(QColor(), QColor(), QColor());
		_queueButton->setCustomColors(QColor(), QColor(), QColor(), QColor());
		_prevButton->setCustomColors(QColor(), QColor(), QColor());
		_nextButton->setCustomColors(QColor(), QColor(), QColor());
		_playPauseButton->setCustomColors(QColor(), QColor(), QColor());

		_progressSlider->setCustomColors(std::nullopt, std::nullopt, std::nullopt);
		_volumeSlider->setCustomColors(std::nullopt, std::nullopt, std::nullopt);

		_orderButton->setIconColorOverride(std::nullopt);
		_repeatButton->setIconColorOverride(std::nullopt);
		_volumeButton->setIconColorOverride(std::nullopt);

		if (_queuePanel) {
			_queuePanel->setCustomColors(theme);
		}
	}
	update();
}

void FullPlayerWidget::transitionToTheme(const CoverTheme &targetTheme, bool animated) {
	if (_targetCoverTheme == targetTheme && (_a_themeTransition.animating() || _coverTheme == targetTheme)) {
		return;
	}

	_targetCoverTheme = targetTheme;
	SetCurrentCoverTheme(targetTheme);

	if (!animated || (!_coverTheme.valid && !targetTheme.valid)) {
		_a_themeTransition.stop();
		applyCoverTheme(targetTheme);
		return;
	}

	_prevCoverTheme = _coverTheme;
	_a_themeTransition.start([=] {
		const auto p = _a_themeTransition.value(1.);
		applyCoverTheme(InterpolateTheme(_prevCoverTheme, _targetCoverTheme, p));
	}, 0., 1., 300);
}

void FullPlayerWidget::updateCoverTheme() {
	const auto useCoverTheme = RabbitSettings::playerCoverColorsFullscreen();
	if (!useCoverTheme || !_currentMedia) {
		transitionToTheme(CoverTheme());
		return;
	}

	const auto image = !_embeddedCover.isNull()
		? _embeddedCover
		: (_currentMedia && _currentMedia->thumbnail())
		? _currentMedia->thumbnail()->original()
		: QImage();

	if (!image.isNull()) {
		const auto isDark = (st::windowBg->c.lightness() < 128);
		const auto newTheme = GenerateCoverTheme(image, isDark);
		if (newTheme.valid) {
			transitionToTheme(newTheme);
			return;
		}
	}

	if (!_hasCover && !_extractingCover) {
		transitionToTheme(CoverTheme());
	}
}

void FullPlayerWidget::updateSongDetails() {
	const auto current = Media::Player::instance()->current(AudioMsgId::Type::Song);
	_currentDocument = current.audio();
	_currentContextId = current.contextId();

	if (!_currentDocument) {
		_songTitle = QString();
		_trackTitle->setText(QString());
		_trackPerformer->setText(QString());
		_headerSubtitle->setText(QString());
		_currentMedia = nullptr;
		_hasCover = false;
		_extractingCover = false;
		_embeddedCover = QImage();
		transitionToTheme(CoverTheme());
		update();
		return;
	}

	const auto name = Ui::Text::FormatSongNameFor(_currentDocument);
	const auto composed = name.composedName();
	_songTitle = composed.title.isEmpty() ? _currentDocument->filename() : composed.title;
	_trackTitle->setText(_songTitle);
	_trackPerformer->setText(composed.performer);

	const auto item = _currentDocument->owner().message(_currentContextId);
	if (item) {
		_headerSubtitle->setText(item->history()->peer->name());
	} else {
		_headerSubtitle->setText(QString());
	}

	_currentMedia = _currentDocument->createMediaView();
	if (_currentMedia) {
		if (!_currentMedia->thumbnail()) {
			const auto origin = item ? item->fullId() : Data::FileOrigin();
			_currentMedia->thumbnailWanted(origin);
		}
		_hasCover = (_currentMedia->thumbnail() != nullptr);
	} else {
		_hasCover = false;
	}

	const auto doc = _currentDocument;
	const auto seq = ++_coverExtractionSeq;
	_extractingCover = true;

	Rabbit::Player::ExtractEmbeddedCover(_currentDocument, crl::guard(this, [=](QImage cover) {
		if (_currentDocument != doc || _coverExtractionSeq != seq) {
			return;
		}
		_extractingCover = false;
		if (!cover.isNull()) {
			_embeddedCover = std::move(cover);
			_hasCover = true;
		} else {
			_hasCover = (_currentMedia && _currentMedia->thumbnail() != nullptr);
			if (!_hasCover) {
				_embeddedCover = QImage();
			}
		}
		updateCoverTheme();
		update();
	}));

	if (_hasCover) {
		_embeddedCover = QImage();
		updateCoverTheme();
	}

	updateTrackState(Media::Player::instance()->getState(AudioMsgId::Type::Song));
	updateControlsGeometry();
	if (_queuePanel && !_queuePanel->isHidden()) {
		_queuePanel->scrollPlaylistToCurrentTrack();
	}
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
		updateCoverTheme();
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
	const auto mode = Core::App().settings().playerRepeatMode();
	switch (mode) {
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
	if (_coverTheme.valid) {
		const auto active = (mode == RepeatMode::One || mode == RepeatMode::All);
		_repeatButton->setIconColorOverride(active ? _coverTheme.accent : _coverTheme.textSecondary);
	} else {
		_repeatButton->setIconColorOverride(std::nullopt);
	}
}

void FullPlayerWidget::updateOrderButton() {
	const auto mode = Core::App().settings().playerOrderMode();
	switch (mode) {
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
	if (_coverTheme.valid) {
		const auto active = (mode == OrderMode::Shuffle || mode == OrderMode::Reverse);
		_orderButton->setIconColorOverride(active ? _coverTheme.accent : _coverTheme.textSecondary);
	} else {
		_orderButton->setIconColorOverride(std::nullopt);
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

	const auto isNarrow = (w < 700);
	const auto isQueueShown = (_queuePanel && !_queuePanel->isHidden());
	const auto queueFullscreen = isQueueShown && isNarrow;

	if (_queuePanel) {
		_queuePanel->setFullscreen(queueFullscreen);
		if (isQueueShown) {
			if (queueFullscreen) {
				_queuePanel->setGeometry(0, 0, w, h);
				_queuePanel->raise();
			} else {
				const auto queueW = _queuePanel->currentWidth();
				_queuePanel->setGeometry(w - queueW, 0, queueW, h);
			}
		}
	}

	const auto queueW = (isQueueShown && !queueFullscreen) ? _queuePanel->currentWidth() : 0;
	const auto availableW = w - queueW;

	const auto topBarY = 14;
	const auto topBarHeight = 34;

	if (availableW < 400) {
		_closeButton->setText(QString());
	} else {
		_closeButton->setText(u"Назад"_q);
	}
	const auto closeW = _closeButton->naturalWidth();
	_closeButton->setGeometry(16, topBarY, closeW, topBarHeight);

	const auto moreW = 34;
	_moreButton->setGeometry(availableW - moreW - 16, topBarY, moreW, topBarHeight);

	const auto sideButtonsWidth = std::max(closeW, moreW) + 16;
	const auto headerTextWidth = std::max(availableW - 2 * sideButtonsWidth - 16, 40);

	if (availableW < 300) {
		_headerSubtitle->hide();
		_headerTitle->resizeToNaturalWidth(headerTextWidth);
		_headerTitle->moveToLeft((availableW - _headerTitle->width()) / 2, topBarY + (topBarHeight - _headerTitle->height()) / 2);
	} else {
		_headerSubtitle->show();
		_headerTitle->resizeToNaturalWidth(headerTextWidth);
		_headerTitle->moveToLeft((availableW - _headerTitle->width()) / 2, topBarY - 2);
		_headerSubtitle->resizeToNaturalWidth(headerTextWidth);
		_headerSubtitle->moveToLeft((availableW - _headerSubtitle->width()) / 2, topBarY + 18);
	}

	const auto isVeryCompactH = (h < 460);
	const auto isCompactH = (h < 580);

	const auto bottomPadding = isVeryCompactH ? 10 : (isCompactH ? 16 : 24);
	const auto volumeY = h - bottomPadding - _volumeButton->height();

	const auto bottomMargin = (availableW < 320) ? 12 : 24;
	const auto bottomBtnHeight = 30;
	const auto bottomBtnWidth = (availableW < 300) ? 46 : (bottomBtnHeight * 2);
	const auto bottomBtnY = volumeY + (_volumeButton->height() - bottomBtnHeight) / 2;

	_speedButton->setGeometry(bottomMargin, bottomBtnY, bottomBtnWidth, bottomBtnHeight);
	_queueButton->setGeometry(availableW - bottomBtnWidth - bottomMargin, bottomBtnY, bottomBtnWidth, bottomBtnHeight);

	const auto roomForVolume = (availableW - bottomBtnWidth - bottomMargin) - (bottomMargin + bottomBtnWidth);
	if (roomForVolume < 130) {
		_volumeSlider->hide();
		_volumeButton->moveToLeft((availableW - _volumeButton->width()) / 2, volumeY);
	} else {
		_volumeSlider->show();
		const auto maxSliderW = isCompactH ? 140 : 180;
		const auto volumeSliderWidth = std::clamp(roomForVolume - _volumeButton->width() - 24, 60, maxSliderW);
		const auto volumeTotalWidth = _volumeButton->width() + 8 + volumeSliderWidth;
		const auto volumeLeft = (availableW - volumeTotalWidth) / 2;
		_volumeButton->moveToLeft(volumeLeft, volumeY);
		_volumeSlider->setGeometry(
			volumeLeft + _volumeButton->width() + 8,
			volumeY + (_volumeButton->height() - 20) / 2,
			volumeSliderWidth,
			20);
	}

	updatePlayPauseGeometry();

	const auto controlsGap = isVeryCompactH ? 48 : (isCompactH ? 56 : 68);
	const auto controlsY = volumeY - controlsGap;

	const auto progressGap = isVeryCompactH ? 38 : (isCompactH ? 44 : 48);
	const auto progressY = controlsY - progressGap;
	const auto progressWidth = std::clamp(availableW - 48, 140, 480);
	const auto progressX = (availableW - progressWidth) / 2;
	_progressSlider->setGeometry(
		progressX,
		progressY,
		progressWidth,
		22);

	_timePassedLabel->resizeToNaturalWidth(70);
	_timeTotalLabel->resizeToNaturalWidth(70);
	_timePassedLabel->moveToLeft(progressX, progressY + 22);
	_timeTotalLabel->moveToLeft(
		progressX + progressWidth - _timeTotalLabel->width(),
		progressY + 22);

	const auto textGap = isVeryCompactH ? 38 : (isCompactH ? 46 : 56);
	const auto textY = progressY - textGap;
	const auto textWidth = std::max(availableW - 40, 80);
	_trackTitle->resizeToNaturalWidth(textWidth);
	_trackTitle->moveToLeft((availableW - _trackTitle->width()) / 2, textY);

	_trackPerformer->resizeToNaturalWidth(textWidth);
	_trackPerformer->moveToLeft((availableW - _trackPerformer->width()) / 2, textY + _trackTitle->height() + 2);

	const auto headerBottom = topBarY + topBarHeight + 8;
	const auto coverAvailableHeight = textY - headerBottom - 12;
	const auto coverAvailableWidth = availableW - 48;
	const auto maxCoverBound = isVeryCompactH ? 140 : (isCompactH ? 220 : 320);
	const auto maxCover = std::min({ coverAvailableWidth, coverAvailableHeight, maxCoverBound });

	if (maxCover < 60) {
		_coverRect = QRect();
	} else {
		const auto coverSize = maxCover;
		const auto coverX = (availableW - coverSize) / 2;
		const auto coverY = headerBottom + (coverAvailableHeight - coverSize) / 2;
		_coverRect = QRect(coverX, coverY, coverSize, coverSize);
	}
}

void FullPlayerWidget::updatePlayPauseGeometry() {
	const auto w = width();
	const auto h = height();
	if (w <= 0 || h <= 0) {
		return;
	}

	const auto isNarrow = (w < 700);
	const auto isQueueShown = (_queuePanel && !_queuePanel->isHidden());
	const auto queueFullscreen = isQueueShown && isNarrow;
	const auto queueW = (isQueueShown && !queueFullscreen) ? _queuePanel->currentWidth() : 0;
	const auto availableW = w - queueW;

	const auto isVeryCompactH = (h < 460);
	const auto isCompactH = (h < 580);
	const auto bottomPadding = isVeryCompactH ? 10 : (isCompactH ? 16 : 24);
	const auto volumeY = h - bottomPadding - _volumeButton->height();
	const auto controlsGap = isVeryCompactH ? 48 : (isCompactH ? 56 : 68);
	const auto controlsY = volumeY - controlsGap;

	const auto playBtnH = isVeryCompactH ? 48 : 60;
	const auto playBaseW = isVeryCompactH ? 48 : 60;
	const auto playExpandedW = isVeryCompactH ? 64 : 80;
	const auto sideBtnSize = isVeryCompactH ? 36 : 44;

	const auto progress = _playPauseButton ? _playPauseButton->playingProgress() : 0.;
	const auto currentPlayWidth = static_cast<int>(std::round(anim::interpolate(playBaseW, playExpandedW, progress)));

	const auto playX = (availableW - currentPlayWidth) / 2;
	const auto playY = controlsY;
	if (_playPauseButton) {
		_playPauseButton->setGeometry(playX, playY, currentPlayWidth, playBtnH);
	}

	if (availableW < 310) {
		if (_orderButton) _orderButton->hide();
		if (_repeatButton) _repeatButton->hide();

		const auto btnSpacing = std::clamp((availableW - currentPlayWidth - 2 * sideBtnSize - 24) / 2, 6, 24);
		const auto prevX = playX - btnSpacing - sideBtnSize;
		if (_prevButton) {
			_prevButton->setGeometry(prevX, playY + (playBtnH - sideBtnSize) / 2, sideBtnSize, sideBtnSize);
		}
		const auto nextX = playX + currentPlayWidth + btnSpacing;
		if (_nextButton) {
			_nextButton->setGeometry(nextX, playY + (playBtnH - sideBtnSize) / 2, sideBtnSize, sideBtnSize);
		}
	} else {
		if (_orderButton) _orderButton->show();
		if (_repeatButton) _repeatButton->show();

		const auto orderW = _orderButton ? _orderButton->width() : 34;
		const auto repeatW = _repeatButton ? _repeatButton->width() : 34;
		const auto neededButtonsW = currentPlayWidth + 2 * sideBtnSize + orderW + repeatW;
		const auto btnSpacing = std::clamp((availableW - neededButtonsW - 24) / 4, 4, 20);

		const auto prevX = playX - btnSpacing - sideBtnSize;
		if (_prevButton) {
			_prevButton->setGeometry(prevX, playY + (playBtnH - sideBtnSize) / 2, sideBtnSize, sideBtnSize);
		}
		if (_orderButton) {
			const auto orderX = prevX - btnSpacing - orderW;
			_orderButton->moveToLeft(orderX, playY + (playBtnH - _orderButton->height()) / 2);
		}

		const auto nextX = playX + currentPlayWidth + btnSpacing;
		if (_nextButton) {
			_nextButton->setGeometry(nextX, playY + (playBtnH - sideBtnSize) / 2, sideBtnSize, sideBtnSize);
		}
		if (_repeatButton) {
			const auto repeatX = nextX + sideBtnSize + btnSpacing;
			_repeatButton->moveToLeft(repeatX, playY + (playBtnH - _repeatButton->height()) / 2);
		}
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

	const auto useCoverTheme = RabbitSettings::playerCoverColorsFullscreen() && _coverTheme.valid;
	if (useCoverTheme) {
		QLinearGradient grad(0, 0, 0, height());
		grad.setColorAt(0.0, _coverTheme.backgroundTop);
		grad.setColorAt(1.0, _coverTheme.backgroundBottom);
		p.fillRect(rect(), grad);
	} else {
		p.fillRect(rect(), st::windowBg);
	}
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
		if (!_embeddedCover.isNull()) {
			PainterHighQualityEnabler hq(p);
			p.drawImage(_coverRect, _embeddedCover);
			drewThumbnail = true;
		} else if (_currentMedia) {
			if (const auto img = _currentMedia->thumbnail()) {
				PainterHighQualityEnabler hq(p);
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
