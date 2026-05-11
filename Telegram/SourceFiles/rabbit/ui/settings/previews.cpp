/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "previews.h"

#include "rabbit/settings/rabbit_settings.h"
#include "styles/style_rabbit_assets.h"
#include "styles/style_chat.h"
#include "ui/painter.h"
#include "ui/chat/chat_style.h"
#include "ui/chat/message_bubble.h"

#include <array>

namespace {

[[nodiscard]] int StickerShapeRadius(int shape) {
	return shape == 1 ? st::bubbleRadiusSmall : st::bubbleRadiusLarge;
}

} // namespace

RoundnessPreview::RoundnessPreview(QWidget *parent) : RpWidget(parent) {
	setMinimumHeight(st::rndPreviewSize);
}

void RoundnessPreview::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto size = st::rndPreviewSize;
	const auto radius = size * (RabbitSettings::userpicRoundness() / 100.);

	p.setPen(Qt::NoPen);
	p.setBrush(st::rndPreviewFill);
	p.drawRoundedRect(0, 0, size, size, radius, radius);

	p.setBrush(st::rndSkeletonFill);
	const auto skeletonLeft = size + st::rndSkeletonLeft;
	const auto skeletonWidth = width() - skeletonLeft;
	const auto skeletonHeight = st::rndSkeletonHeight;
	const auto skeletonRadius = skeletonHeight / 2.;
	p.drawRoundedRect(
		skeletonLeft,
		skeletonHeight,
		skeletonWidth / 2,
		skeletonHeight,
		skeletonRadius,
		skeletonRadius);

	p.drawRoundedRect(
		skeletonLeft,
		skeletonHeight * 3,
		skeletonWidth / 2,
		skeletonHeight,
		skeletonRadius,
		skeletonRadius);
}

ChatPreview::ChatPreview(QWidget *parent) : RpWidget(parent) {
	setMinimumHeight(st::stickerPreviewSize);
}

void ChatPreview::ensureChatStyle() {
	const auto version = style::PaletteVersion();
	if (_chatStyle && _paletteVersion == version) {
		return;
	}
	_paletteVersion = version;
	_chatStyle = std::make_unique<Ui::ChatStyle>(
		style::main_palette::get());
}

void ChatPreview::paintEvent(QPaintEvent *e) {
	ensureChatStyle();

	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto stickerSize = RabbitSettings::stickerSize();
	const auto stickerRect = QSize(stickerSize, static_cast<int>(stickerSize * 0.7));
	const auto stickerShape = RabbitSettings::stickerShape();
	const auto stickerRadius = static_cast<qreal>(StickerShapeRadius(stickerShape));

	p.setPen(Qt::NoPen);
	p.setBrush(st::rndPreviewFill);
	p.drawRoundedRect(
		QRect(QPoint(0, 0), stickerRect),
		stickerRadius,
		stickerRadius);

	const auto timeRadius = st::stickerPreviewTimeHeight / 2.;
	p.setBrush(st::rndSkeletonFill);
	p.drawRoundedRect(
		stickerRect.width() + st::stickerPreviewMargin,
		stickerRect.height() - st::stickerPreviewTimeHeight,
		st::stickerPreviewTimeWidth,
		st::stickerPreviewTimeHeight,
		timeRadius,
		timeRadius);

	struct BubbleEntry {
		bool outgoing;
		double widthFraction;
	};
	constexpr auto bubbles = std::array<BubbleEntry, 4>{ {
		{ .outgoing = true, .widthFraction = 0.85 },
		{ .outgoing = false, .widthFraction = 0.6 },
		{ .outgoing = true, .widthFraction = 0.5 },
		{ .outgoing = false, .widthFraction = 0.7 },
	} };

	const auto bubbleHeight = st::stickerSpacefillerHeight;
	const auto outerWidth = width();
	const auto tailWidth = st::historyBubbleTailInLeft.width();
	auto topOffset = stickerRect.height() + st::stickerPreviewMargin;

	for (const auto &entry : bubbles) {
		const auto bubbleWidth = static_cast<int>(outerWidth * 0.5 * entry.widthFraction);
		const auto bubbleLeft = entry.outgoing
			? outerWidth - bubbleWidth - tailWidth
			: tailWidth;

		Ui::BubbleRounding rounding;
		rounding.topLeft = Ui::BubbleCornerRounding::Large;
		rounding.topRight = Ui::BubbleCornerRounding::Large;
		if (entry.outgoing) {
			rounding.bottomLeft = Ui::BubbleCornerRounding::Large;
			rounding.bottomRight = Ui::BubbleCornerRounding::Tail;
		} else {
			rounding.bottomLeft = Ui::BubbleCornerRounding::Tail;
			rounding.bottomRight = Ui::BubbleCornerRounding::Large;
		}

		Ui::PaintBubble(p, Ui::SimpleBubble{
			.st = _chatStyle.get(),
			.geometry = QRect(bubbleLeft, topOffset, bubbleWidth, bubbleHeight),
			.patternViewport = QRect(),
			.outerWidth = outerWidth,
			.outbg = entry.outgoing,
			.rounding = rounding,
		});

		topOffset += st::stickerPreviewMargin + bubbleHeight + st::msgShadow;
	}
}

StickerShapePicker::StickerShapePicker(QWidget *parent) : RpWidget(parent) {
	setMinimumSize(st::stickerShapeBoxWidth, st::stickerShapeBoxHeight);
	setCursor(style::cur_pointer);
}

void StickerShapePicker::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto activePen = QPen(st::windowBgActive, st::stickerShapePenWidth);
	const auto inactivePen = QPen(st::rndSkeletonFill, st::stickerShapePenWidth);
	const auto activeShape = RabbitSettings::stickerShape();

	const auto cardWidth = st::stickerShapeVariantCardWidth;
	const auto cardHeight = st::stickerShapeVariantCardHeight;
	const auto cardMargin = st::stickerShapeMargins;
	const auto padding = st::stickerShapePadding;
	const auto pen = st::stickerShapePenWidth;

	const auto innerWidth = cardWidth - 2 * padding - 2 * pen;
	const auto innerHeight = cardHeight - 2 * padding;

	auto cardLeft = pen;
	for (auto i = 0; i < 3; ++i) {
		p.setPen((activeShape == i) ? activePen : inactivePen);
		p.setBrush(Qt::NoBrush);
		p.drawRoundedRect(
			cardLeft,
			pen,
			cardWidth,
			cardHeight,
			st::bubbleRadiusSmall,
			st::bubbleRadiusSmall);

		const auto radius = StickerShapeRadius(i);
		p.setPen(Qt::NoPen);
		p.setBrush(st::rndPreviewFill);
		p.drawRoundedRect(
			cardLeft + padding + pen,
			padding + pen,
			innerWidth,
			innerHeight,
			radius,
			radius);

		cardLeft += cardWidth + cardMargin;
	}
}

void StickerShapePicker::mousePressEvent(QMouseEvent *e) {
	const auto cardWidth = st::stickerShapeVariantCardWidth;
	const auto cardMargin = st::stickerShapeMargins;
	const auto cellWidth = cardWidth + cardMargin;
	const auto x = e->pos().x() - st::stickerShapePenWidth;
	if (x < 0) {
		return;
	}
	const auto index = std::min(static_cast<int>(x / cellWidth), 2);
	if (index != RabbitSettings::stickerShape()) {
		RabbitSettings::setStickerShape(index);
		update();
	}
}

QuickActionsPreview::QuickActionsPreview(QWidget *parent) : RpWidget(parent) {
	setMinimumSize(st::quickActionPreviewWidth, st::quickActionPreviewHeight);
}

void QuickActionsPreview::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);
	
	const auto radius = st::quickActionBoxHeight / 4;

	p.setPen(QPen(st::quickActionBoxStroke, 1.));
	p.setBrush(st::windowBgOver);
	p.drawRoundedRect(
		1, 
		1, 
		st::quickActionBoxWidth, 
		st::quickActionBoxHeight, 
		radius, 
		radius);
	
	p.drawRoundedRect(
		st::quickActionPreviewWidth - st::quickActionBoxWidth - 1, 
		st::quickActionPreviewHeight - st::quickActionBoxHeight - 1, 
		st::quickActionBoxWidth, 
		st::quickActionBoxHeight, 
		radius, 
		radius);
	
	// TODO: implement drawing current icons
}
