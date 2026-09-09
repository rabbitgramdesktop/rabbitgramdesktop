/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "info/profile/info_profile_music_button.h"

#include "ui/color_contrast.h"
#include "ui/effects/animation_value.h"
#include "ui/text/text_utilities.h"
#include "ui/painter.h"
#include "ui/rect.h"
#include "ui/ui_utility.h"
#include "styles/style_chat.h"
#include "styles/style_info.h"

namespace Info::Profile {

MusicButton::MusicButton(
	QWidget *parent,
	MusicButtonData data,
	Fn<void()> handler)
: RippleButton(parent, st::infoMusicButtonRipple)
, _noteSymbol(u"\u266B"_q + QChar(' '))
, _noteWidth(st::normalFont->width(_noteSymbol)) {
	updateData(std::move(data));
	setClickedCallback(std::move(handler));
}

MusicButton::~MusicButton() = default;

void MusicButton::updateData(MusicButtonData data) {
	const auto result = data.name.textWithEntities();
	const auto performerLength = result.entities.empty()
		? 0
		: int(result.entities.front().length());
	_title.setText(
		st::semiboldTextStyle,
		result.text.mid(3 + performerLength, result.text.size()));
	_performer.setText(
		st::defaultTextStyle,
		result.text.mid(0, performerLength));
	setAccessibleName(result.text);
	update();
}

void MusicButton::setOverrideBg(std::optional<QColor> color) {
	_overrideBg = color;
	update();
}

void MusicButton::paintEvent(QPaintEvent *e) {
	auto p = QPainter(this);

	if (_overrideBg) {
		p.fillRect(e->rect(), Ui::BlendColors(
			*_overrideBg,
			Qt::black,
			st::infoProfileTopBarActionButtonBgOpacity));
	} else {
		p.fillRect(e->rect(), st::shadowFg);
	}
	const auto lightBackground = _overrideBg
		&& Ui::IsLightBackground(*_overrideBg);
	const auto rippleColor = !_overrideBg
		? std::nullopt
		: std::make_optional(anim::with_alpha(
			(lightBackground
				? QColor(Qt::black)
				: st::groupCallMembersFg->c),
			st::infoProfileTopBarBackdropRippleOpacity));
	paintRipple(p, QPoint(), rippleColor ? &*rippleColor : nullptr);

	const auto padding = st::infoMusicButtonPadding;
	const auto contentLeft = padding.left();
	const auto &icon = st::topicButtonArrow;
	const auto iconWidth = icon.width();
	const auto iconHeight = icon.height();
	const auto contentWidth = width() - padding.left() - padding.right() - iconWidth - padding.right();

	// title part
	const auto titleFont = st::semiboldFont;
	const auto titleTop = padding.top();

	p.setPen(_overrideBg ? st::groupCallMembersFg : st::windowBoldFg);
	p.setFont(titleFont);
	
	_title.draw(p, {
		.position = QPoint(
			contentLeft,
			titleTop),
		.availableWidth = contentWidth,
		.now = crl::now(),
		.elisionLines = 1,
	});

	// performer part
	const auto performerFont = st::normalFont;
	const auto performerTop = titleTop + titleFont->height + st::infoMusicButtonMargin;

	p.setPen(_overrideBg ? st::groupCallVideoSubTextFg : st::windowSubTextFg);
	p.setFont(performerFont);

	_performer.draw(p, {
		.position = { contentLeft, performerTop },
		.availableWidth = contentWidth,
		.now = crl::now(),
		.elisionLines = 1,
	});

	// icon part
	const auto iconTop = (height() - iconHeight) / 2.0;
	const auto iconLeft = width() - padding.right() - iconWidth;

	icon.paint(p, iconLeft,	iconTop, iconWidth, p.pen().color());
}

int MusicButton::resizeGetHeight(int newWidth) {
	const auto padding = st::infoMusicButtonPadding;
	const auto &font = st::defaultTextStyle.font;

	return padding.top() 
		+ font->height 
		+ st::infoMusicButtonMargin
		+ font->height
		+ padding.bottom();
}

} // namespace Info::Profile
