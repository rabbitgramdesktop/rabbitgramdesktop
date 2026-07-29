/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/settings/previews.h"

#include "rabbit/settings/quick_action.h"
#include "rabbit/settings/quick_action_ui.h"
#include "rabbit/settings/rabbit_settings.h"
#include "ui/chat/chat_style.h"
#include "ui/chat/message_bubble.h"
#include "ui/painter.h"

#include <array>
#include <rpl/producer.h>

#include "styles/style_chat.h"
#include "styles/style_chat_style.h"
#include "styles/style_rabbit_assets.h"

#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_helpers.h"
#include "data/data_session.h"
#include "data/data_peer.h"
#include "main/main_session.h"
#include "window/window_session_controller.h"
#include "lang/lang_keys.h"
#include "ui/empty_userpic.h"
#include "ui/chat/chat_theme.h"
#include "ui/image/image_prepare.h"
#include "base/unixtime.h"
#include "window/section_widget.h"

namespace {

[[nodiscard]] AdminLog::OwnedItem GenerateTextItem(
		not_null<HistoryView::ElementDelegate*> delegate,
		not_null<History*> history,
		const QString &text,
		bool out) {
	Expects(history->peer->isUser());

	const auto item = history->makeMessage({
		.id = history->nextNonHistoryEntryId(),
		.flags = (MessageFlag::FakeHistoryItem
			| MessageFlag::HasFromId
			| (out ? MessageFlag::Outgoing : MessageFlag(0))),
		.from = (out
			? history->session().userId()
			: peerToUser(history->peer->id)),
		.date = base::unixtime::now(),
	}, TextWithEntities{ text }, MTP_messageMediaEmpty());
	return AdminLog::OwnedItem(delegate, item);
}

[[nodiscard]] int StickerShapeRadius(int shape) {
	switch (shape) {
	case 1: return st::bubbleRadiusSmall;
	case 2: return st::bubbleRadiusLarge;
	default: return 0;
	}
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

ChatPreview::ChatPreview(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: RpWidget(parent)
, SimpleElementDelegate(controller, [=] { update(); })
, _controller(controller)
, _history(controller->session().data().history(PeerData::kServiceNotificationsId))
, _item1(GenerateTextItem(delegate(), _history, u"Hello! This is a slightly longer message to show how bubbles look."_q, true))
, _item2(GenerateTextItem(delegate(), _history, u"Oh, I see. Looks good!"_q, false))
, _item3(GenerateTextItem(delegate(), _history, u"Right?"_q, true))
	, _item4(GenerateTextItem(delegate(), _history, u"Yes, and the rounding looks great too."_q, false))
{
	setMinimumHeight(st::stickerPreviewSize);

	rpl::merge(
		RabbitSettings::JsonSettings::Events(u"message_roundness"_q),
		RabbitSettings::JsonSettings::Events(u"hide_bubble_tails"_q),
		RabbitSettings::JsonSettings::Events(u"show_online_indicator"_q)
	) | rpl::on_next([=] {
		update();
	}, lifetime());
}

not_null<HistoryView::ElementDelegate*> ChatPreview::delegate() {
	return static_cast<HistoryView::ElementDelegate*>(this);
}

HistoryView::Context ChatPreview::elementContext() {
	return HistoryView::Context::ContactPreview;
}

void ChatPreview::paintEvent(QPaintEvent *e) {
	const auto version = style::PaletteVersion();
	const auto roundness = RabbitSettings::messageRoundness();
	if (!_chatStyle || _paletteVersion != version || _messageRoundness != roundness) {
		_paletteVersion = version;
		_messageRoundness = roundness;
		_chatStyle = std::make_unique<Ui::ChatStyle>(
			style::main_palette::get());
	}

	const auto ratio = style::DevicePixelRatio();
	auto image = QImage(
		QSize(width(), height()) * ratio,
		QImage::Format_ARGB32_Premultiplied);
	image.setDevicePixelRatio(ratio);
	image.fill(Qt::transparent);

	auto topOffset = st::stickerPreviewMargin;

	{
		Painter p(&image);
		PainterHighQualityEnabler hq(p);

		Window::SectionWidget::PaintBackground(
			p,
			_controller->defaultChatTheme().get(),
			QSize(width(), height()),
			QRect(QPoint(), size()));

		auto context = _controller->defaultChatTheme()->preparePaintContext(
			_chatStyle.get(),
			QRect(QPoint(), size()),
			QRect(QPoint(), size()),
			QRect(QPoint(), size()),
			_controller->isGifPausedAtLeastFor(Window::GifPauseReason::Layer));

		const auto paintUserpic = [&](
				Painter &target,
				int userpicLeft,
				int userpicTop,
				int outerWidth) {
			Ui::EmptyUserpic emptyUserpic(
				Ui::EmptyUserpic::UserpicColor(0),
				u"U"_q);
			emptyUserpic.paintCircle(
				target,
				userpicLeft,
				userpicTop,
				outerWidth,
				st::msgPhotoSize);
		};

		const auto paintOnlineIndicator = [&](
				Painter &target,
				int userpicLeft,
				int userpicTop) {
			const auto dotDiameter = 10.0;
			const auto borderWidth = 2.0;
			const auto totalSize = dotDiameter + borderWidth * 2;
			const auto dotX = userpicLeft + st::msgPhotoSize - totalSize + borderWidth;
			const auto dotY = userpicTop + st::msgPhotoSize - totalSize + borderWidth;
			const auto outlineRect = QRectF(
				dotX - borderWidth,
				dotY - borderWidth,
				totalSize,
				totalSize);
			const auto dotRect = QRectF(dotX, dotY, dotDiameter, dotDiameter);

			auto userpicImage = QImage(
				QSize(st::msgPhotoSize, st::msgPhotoSize) * ratio,
				QImage::Format_ARGB32_Premultiplied);
			userpicImage.setDevicePixelRatio(ratio);
			userpicImage.fill(Qt::transparent);
			{
				Painter q(&userpicImage);
				PainterHighQualityEnabler hq(q);
				paintUserpic(q, 0, 0, st::msgPhotoSize);
				q.setPen(Qt::NoPen);
				q.setCompositionMode(QPainter::CompositionMode_Clear);
				q.setBrush(Qt::transparent);
				q.drawEllipse(outlineRect.translated(
					-userpicLeft,
					-userpicTop));
				q.setCompositionMode(QPainter::CompositionMode_SourceOver);
				q.setBrush(st::dialogsOnlineBadgeFg);
				q.drawEllipse(dotRect.translated(
					-userpicLeft,
					-userpicTop));
			}
			target.drawImage(userpicLeft, userpicTop, userpicImage);
		};

		for (auto item : { &_item1, &_item2, &_item3, &_item4 }) {
			const auto isIncoming = !(*item)->hasOutLayout();
			const auto photoSkip = isIncoming ? st::msgPhotoSkip : 0;

			(*item)->initDimensions();
			(*item)->resizeGetHeight(width() - photoSkip);

			context.outbg = !isIncoming;
			p.translate(photoSkip, topOffset);
			(*item)->draw(p, context);
			p.translate(-photoSkip, -topOffset);

			if (isIncoming) {
				const auto userpicLeft = st::historyPhotoLeft;
				const auto userpicTop = topOffset
					+ (*item)->height()
					- st::msgPhotoSize;

				if (RabbitSettings::showOnlineIndicator()) {
					paintOnlineIndicator(p, userpicLeft, userpicTop);
				} else {
					paintUserpic(p, userpicLeft, userpicTop, width());
				}
			}

			topOffset += (*item)->height() + st::msgMargin.bottom();
		}


	}

	const auto rounded = Images::Round(
		std::move(image),
		Images::CornersMask(ImageRoundRadius::Large));

	QPainter pw(this);
	pw.drawImage(QPoint(), rounded);

	if (height() != topOffset) {
		setMinimumHeight(topOffset);
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

	RabbitSettings::JsonSettings::Events(
		u"incoming_quick_action"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	RabbitSettings::JsonSettings::Events(
		u"outgoing_quick_action"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());
}

void QuickActionsPreview::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto radius = st::quickActionBoxHeight / 4;
	const auto incomingBox = QRect(
		QPoint(1, 1),
		QSize(st::quickActionBoxWidth, st::quickActionBoxHeight));
	const auto outgoingBox = QRect(
		QPoint(
			st::quickActionPreviewWidth - st::quickActionBoxWidth - 1,
			st::quickActionPreviewHeight - st::quickActionBoxHeight - 1),
		QSize(st::quickActionBoxWidth, st::quickActionBoxHeight));

	p.setPen(QPen(st::quickActionBoxStroke, 1.));
	p.setBrush(st::windowBgOver);
	p.drawRoundedRect(
		incomingBox,
		radius,
		radius);

	p.drawRoundedRect(
		outgoingBox,
		radius,
		radius);

	const auto incomingAction = static_cast<RabbitSettings::QuickAction>(RabbitSettings::incomingQuickAction());
	const auto outgoingAction = static_cast<RabbitSettings::QuickAction>(RabbitSettings::outgoingQuickAction());
	auto incomingIcon = RabbitSettings::QuickActionIcon(incomingAction);
	auto outgoingIcon = RabbitSettings::QuickActionIcon(outgoingAction);

	if (incomingIcon) {
		const auto icon = Settings::Icon(std::move(incomingIcon));
		const auto iconLeft = incomingBox.x()
			+ (incomingBox.width() - icon.width()) / 2;
		const auto iconTop = incomingBox.y()
			+ (incomingBox.height() - icon.height()) / 2;
		icon.paint(p, iconLeft, iconTop);
	}

	if (outgoingIcon) {
		const auto icon = Settings::Icon(std::move(outgoingIcon));
		const auto iconLeft = outgoingBox.x()
			+ (outgoingBox.width() - icon.width()) / 2;
		const auto iconTop = outgoingBox.y()
			+ (outgoingBox.height() - icon.height()) / 2;
		icon.paint(p, iconLeft, iconTop);
	}
}
