#include <QtCore/QFile>
/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/settings/previews.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "rabbit/quick_actions/quick_action.h"
#include "rabbit/quick_actions/quick_action_ui.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "ui/chat/chat_style.h"
#include "ui/chat/message_bubble.h"
#include "ui/filter_icons.h"
#include "ui/painter.h"
#include "ui/unread_badge_paint.h"

#include <array>
#include <rpl/producer.h>

#include "styles/style_chat.h"
#include "styles/style_chat_style.h"
#include "styles/style_dialogs.h"
#include "styles/style_rabbit_assets.h"
#include "styles/style_window.h"

#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_helpers.h"
#include "data/data_session.h"
#include "data/data_peer.h"
#include "data/data_document.h"
#include "data/data_document_media.h"
#include "data/data_user.h"
#include "data/data_chat_filters.h"
#include "main/main_session.h"
#include "window/window_session_controller.h"
#include "lang/lang_keys.h"
#include "ui/empty_userpic.h"
#include "ui/chat/chat_theme.h"
#include "ui/image/image_prepare.h"
#include "base/unixtime.h"
#include "window/section_widget.h"

namespace {

void UpdatePreviewUser(not_null<UserData*> user) {
	user->setLoadedStatus(PeerData::LoadedStatus::Full);
	user->setName(ktr(u"rtg_chats_preview_user"_q), QString(), QString(), QString());
}

[[nodiscard]] not_null<UserData*> CreatePreviewUser(
		not_null<Window::SessionController*> controller) {
	const auto user = controller->session().data().user(UserId(123456789));
	UpdatePreviewUser(user);
	return user;
}

[[nodiscard]] QByteArray LoadStickerBytes() {
	for (const auto &path : {
		u":/gui/art/rabbit/sticker.webp"_q,
		u"Telegram/Resources/art/rabbit/sticker.webp"_q,
		u"/home/xmdnusr/source/rtgdesktopdev/Telegram/Resources/art/rabbit/sticker.webp"_q,
		u"/home/xmdnusr/Downloads/sticker.webp"_q,
	}) {
		auto file = QFile(path);
		if (file.open(QIODevice::ReadOnly)) {
			const auto data = file.readAll();
			if (!data.isEmpty()) {
				return data;
			}
		}
	}
	return QByteArray();
}

[[nodiscard]] not_null<DocumentData*> CreatePreviewSticker(
		not_null<Main::Session*> session,
		std::shared_ptr<Data::DocumentMedia> &mediaViewHolder) {
	static const auto kBytes = LoadStickerBytes();
	const auto doc = session->data().document(
		DocumentId(0x7fffffff12345678ULL),
		0,
		QByteArray(),
		base::unixtime::now(),
		QVector<MTPDocumentAttribute>{
			MTP_documentAttributeImageSize(MTP_int(512), MTP_int(512)),
			MTP_documentAttributeSticker(
				MTP_flags(0),
				MTP_string(u"👍"_q),
				MTP_inputStickerSetShortName(MTP_string(u"HmdnPack"_q)),
				MTPMaskCoords()),
		},
		u"image/webp"_q,
		InlineImageLocation(),
		ImageWithLocation(),
		ImageWithLocation(),
		false,
		1,
		int64(kBytes.size()));

	if (!mediaViewHolder) {
		const auto fileLoc = QFile::exists(u":/gui/art/rabbit/sticker.webp"_q)
			? u":/gui/art/rabbit/sticker.webp"_q
			: QFile::exists(u"/home/xmdnusr/Downloads/sticker.webp"_q)
			? u"/home/xmdnusr/Downloads/sticker.webp"_q
			: u"Telegram/Resources/art/rabbit/sticker.webp"_q;
		doc->setLocation(Core::FileLocation(fileLoc));
		mediaViewHolder = doc->createMediaView();
		mediaViewHolder->setBytes(kBytes);
		auto img = Images::Read({ .content = kBytes }).image;
		if (img.isNull()) {
			img.loadFromData(kBytes, "WEBP");
		}
		if (!img.isNull()) {
			mediaViewHolder->goodThumbnailWanted();
			mediaViewHolder->setGoodThumbnail(img);
			mediaViewHolder->setThumbnail(img);
		}
		mediaViewHolder->checkStickerLarge();
	}
	return doc;
}

[[nodiscard]] AdminLog::OwnedItem GenerateTextItem(
		not_null<HistoryView::ElementDelegate*> delegate,
		not_null<History*> history,
		const QString &text,
		bool out,
		HistoryItem *replyTo = nullptr) {
	Expects(history->peer->isUser());

	auto flags = MessageFlag::FakeHistoryItem
		| MessageFlag::HasFromId
		| (out ? MessageFlag::Outgoing : MessageFlag(0));
	if (replyTo) {
		flags |= MessageFlag::HasReplyInfo;
	}
	const auto item = history->makeMessage({
		.id = history->nextNonHistoryEntryId(),
		.flags = flags,
		.from = (out
			? history->session().userId()
			: peerToUser(history->peer->id)),
		.replyTo = replyTo
			? FullReplyTo{ .messageId = replyTo->fullId() }
			: FullReplyTo(),
		.date = base::unixtime::now(),
	}, TextWithEntities{ text }, MTP_messageMediaEmpty());
	return AdminLog::OwnedItem(delegate, item);
}

[[nodiscard]] AdminLog::OwnedItem GenerateStickerItem(
		not_null<HistoryView::ElementDelegate*> delegate,
		not_null<History*> history,
		not_null<DocumentData*> sticker,
		bool out,
		HistoryItem *replyTo = nullptr) {
	Expects(history->peer->isUser());

	auto flags = MessageFlag::FakeHistoryItem
		| MessageFlag::HasFromId
		| (out ? MessageFlag::Outgoing : MessageFlag(0));
	if (replyTo) {
		flags |= MessageFlag::HasReplyInfo;
	}
	const auto item = history->makeMessage({
		.id = history->nextNonHistoryEntryId(),
		.flags = flags,
		.from = (out
			? history->session().userId()
			: peerToUser(history->peer->id)),
		.replyTo = replyTo
			? FullReplyTo{ .messageId = replyTo->fullId() }
			: FullReplyTo(),
		.date = base::unixtime::now(),
	}, sticker, TextWithEntities());
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
, _history(controller->session().data().history(CreatePreviewUser(controller)))
{
	regenerateItems();
	setMinimumHeight(st::stickerPreviewSize);

	rpl::merge(
		RabbitSettings::JsonSettings::Events(u"sticker_size"_q),
		RabbitSettings::JsonSettings::Events(u"sticker_shape"_q)
	) | rpl::on_next([=] {
		updateItems();
	}, lifetime());

	rpl::merge(
		RabbitSettings::JsonSettings::Events(u"message_roundness"_q),
		RabbitSettings::JsonSettings::Events(u"hide_bubble_tails"_q),
		RabbitSettings::JsonSettings::Events(u"show_online_indicator"_q)
	) | rpl::on_next([=] {
		updateGeometry();
		update();
	}, lifetime());

	RabbitLang::Lang::Events(
	) | rpl::on_next([=] {
		updateItems();
	}, lifetime());
}

void ChatPreview::regenerateItems() {
	_item0 = nullptr;
	_item1 = nullptr;
	_item2 = nullptr;

	if (const auto user = _history->peer->asUser()) {
		UpdatePreviewUser(user);
	}

	_item0 = GenerateTextItem(
		delegate(),
		_history,
		ktr(u"rtg_chats_preview_message1"_q),
		false);
	_item1 = GenerateStickerItem(
		delegate(),
		_history,
		CreatePreviewSticker(&_controller->session(), _stickerMedia),
		true,
		_item0->data());
	_item2 = GenerateTextItem(
		delegate(),
		_history,
		ktr(u"rtg_chats_preview_message2"_q),
		false,
		_item1->data());
}

void ChatPreview::updateItems() {
	regenerateItems();
	if (width() > 0) {
		const auto newHeight = resizeGetHeight(width());
		if (height() != newHeight) {
			resize(width(), newHeight);
		}
	}
	update();
}

int ChatPreview::resizeGetHeight(int newWidth) {
	if (!_item1 || !_item2) {
		return st::stickerPreviewSize;
	}
	auto result = st::stickerPreviewMargin;
	for (auto item : { &_item1, &_item2 }) {
		const auto isIncoming = !(*item)->hasOutLayout();
		const auto photoSkip = isIncoming ? st::msgPhotoSkip : 0;
		(*item)->initDimensions();
		(*item)->resizeGetHeight(newWidth - photoSkip);
		result += (*item)->height() + st::msgMargin.bottom();
	}
	result += st::stickerPreviewMargin;
	return std::max(result, int(st::stickerPreviewSize));
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

		const auto userpicLetter = [&] {
			const auto name = _history->peer->name();
			return name.isEmpty() ? u"U"_q : name.left(1);
		}();
		const auto paintUserpic = [&](
				Painter &target,
				int userpicLeft,
				int userpicTop,
				int outerWidth) {
			Ui::EmptyUserpic emptyUserpic(
				Ui::EmptyUserpic::UserpicColor(0),
				userpicLetter);
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

		if (!_item1 || !_item2) {
			return;
		}

		for (auto item : { &_item1, &_item2 }) {
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

FoldersPreview::FoldersPreview(
	QWidget *parent,
	Window::SessionController *controller)
: RpWidget(parent)
, _controller(controller) {
	setMinimumHeight(220);

	if (_controller) {
		const auto filters = &_controller->session().data().chatsFilters();
		filters->changed(
		) | rpl::on_next([=] {
			update();
		}, lifetime());
	}

	RabbitSettings::JsonSettings::Events(
		u"folders_style"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	RabbitSettings::JsonSettings::Events(
		u"folders_show_icon"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	RabbitSettings::JsonSettings::Events(
		u"folders_hide_all_chats"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	RabbitSettings::JsonSettings::Events(
		u"folders_unread_counter"_q
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	Core::App().settings().chatFiltersHorizontalChanges(
	) | rpl::on_next([=] {
		update();
	}, lifetime());

	style::PaletteChanged(
	) | rpl::on_next([=] {
		update();
	}, lifetime());
}

int FoldersPreview::resizeGetHeight(int newWidth) {
	return 220;
}

void FoldersPreview::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto cardRect = QRect(0, 0, width(), height());
	const auto radius = 8.;

	auto clipPath = QPainterPath();
	clipPath.addRoundedRect(cardRect, radius, radius);
	p.setClipPath(clipPath);

	p.fillRect(cardRect, st::windowBg);

	struct TabData {
		QString name;
		Ui::FilterIcon icon;
		int unread = 0;
		bool muted = false;
	};
	const auto hideAll = RabbitSettings::foldersHideAllChats();
	auto tabs = std::vector<TabData>();

	if (_controller) {
		const auto session = &_controller->session();
		const auto &list = session->data().chatsFilters().list();
		for (const auto &filter : list) {
			if (hideAll && !filter.id()) {
				continue;
			}
			const auto name = !filter.id()
				? tr::lng_filters_all(tr::now)
				: filter.titleText().text;
			const auto icon = !filter.id()
				? Ui::FilterIcon::All
				: Ui::ComputeFilterIcon(filter);
			const auto unread = !filter.id()
				? session->data().chatsList()->unreadState()
				: session->data().chatsFilters().chatsList(filter.id())->unreadState();
			const auto count = unread.chats + unread.marks;
			const auto isMuted = (count <= (unread.chatsMuted + unread.marksMuted));
			tabs.push_back({ name, icon, count, isMuted });
		}
	}

	if (tabs.empty() || (tabs.size() == 1 && !hideAll)) {
		tabs.clear();
		if (!hideAll) {
			tabs.push_back({ tr::lng_filters_all(tr::now), Ui::FilterIcon::All, 12, false });
		}
		tabs.push_back({ tr::lng_filters_name_people(tr::now), Ui::FilterIcon::Private, 3, true });
		tabs.push_back({ tr::lng_filters_include_channels(tr::now), Ui::FilterIcon::Channels, 0, false });
		if (hideAll) {
			tabs.push_back({ tr::lng_filters_name_unread(tr::now), Ui::FilterIcon::Unread, 5, false });
		}
	}

	if (_activeTab >= int(tabs.size())) {
		_activeTab = 0;
	}

	const auto counterMode = RabbitSettings::foldersUnreadCounter();
	const auto shouldShowBadge = [=](const TabData &tab) {
		if (counterMode == 2 || tab.unread <= 0) {
			return false;
		}
		if (counterMode == 1 && tab.muted) {
			return false;
		}
		return true;
	};

	const auto horizontal = Core::App().settings().chatFiltersHorizontal();
	const auto foldersStyle = RabbitSettings::foldersStyle();

	if (horizontal) {
		const auto tabsHeight = 38;
		p.fillRect(0, 0, width(), tabsHeight, st::windowBgOver);
		p.fillRect(0, tabsHeight - 1, width(), 1, st::shadowFg);

		const auto font = st::dialogsSearchTabs.labelStyle.font;
		const auto showIcon = RabbitSettings::foldersShowIcon();
		const auto iconHeight = 20;

		auto unreadSt = Ui::UnreadBadgeStyle();
		unreadSt.align = style::al_left;

		p.save();
		p.setClipRect(0, 0, width(), tabsHeight);

		auto currentX = 10;
		const auto count = std::min(int(tabs.size()), 3);
		for (auto i = 0; i < count; ++i) {
			const auto &tab = tabs[i];
			const auto active = (i == _activeTab);

			auto contentWidth = font->width(tab.name);
			auto iconWidth = 0;
			if (showIcon) {
				const auto &icons = Ui::LookupFilterIcon(tab.icon);
				const auto icon = active ? icons.active : icons.normal;
				iconWidth = (icon->height() > 0)
					? int(std::round(icon->width() * (double(iconHeight) / icon->height())))
					: icon->width();
				contentWidth += iconWidth + 6;
			}
			auto badgeWidth = 0;
			if (shouldShowBadge(tab)) {
				unreadSt.muted = tab.muted;
				badgeWidth = Ui::CountUnreadBadgeSize(QString::number(tab.unread), unreadSt).width() + 5;
				contentWidth += badgeWidth;
			}

			const auto tabPadding = 8;
			const auto tabWidth = contentWidth + tabPadding * 2;
			if (currentX + tabWidth > width() - 8 && i > 0) {
				break;
			}
			const auto tabRect = QRect(currentX, 0, tabWidth, tabsHeight);

			if (active) {
				p.setPen(Qt::NoPen);
				if (foldersStyle == 0) {
					p.setBrush(st::dialogsSearchTabs.rippleBgActive->c);
					p.drawRoundedRect(tabRect.x(), 4, tabRect.width(), tabsHeight - 8, 4, 4);
				} else if (foldersStyle == 1) {
					p.setBrush(st::dialogsSearchTabs.labelFgActive->c);
					p.drawRoundedRect(tabRect.x() + 4, tabsHeight - 4, tabRect.width() - 8, 3, 1.5, 1.5);
				} else if (foldersStyle == 2) {
					p.setBrush(st::dialogsSearchTabs.rippleBgActive->c);
					const auto h = tabsHeight - 10;
					p.drawRoundedRect(tabRect.x(), 5, tabRect.width(), h, h * 0.25, h * 0.25);
				} else if (foldersStyle == 3) {
					p.setBrush(st::dialogsSearchTabs.rippleBgActive->c);
					const auto h = tabsHeight - 10;
					p.drawRoundedRect(tabRect.x(), 5, tabRect.width(), h, h * 0.5, h * 0.5);
				}
			}

			auto itemX = currentX + tabPadding;
			if (showIcon) {
				const auto &icons = Ui::LookupFilterIcon(tab.icon);
				const auto icon = active ? icons.active : icons.normal;
				p.save();
				p.translate(itemX, (tabsHeight - iconHeight) / 2);
				const auto scale = double(iconHeight) / icon->height();
				p.scale(scale, scale);
				icon->paint(p, 0, 0, icon->width());
				p.restore();
				itemX += iconWidth + 6;
			}

			p.setFont(font);
			p.setPen(active ? st::dialogsSearchTabs.labelFgActive : st::dialogsSearchTabs.labelFg);
			p.drawText(itemX, (tabsHeight - font->height) / 2 + font->ascent, tab.name);
			itemX += font->width(tab.name);

			if (shouldShowBadge(tab)) {
				unreadSt.muted = tab.muted;
				Ui::PaintUnreadBadge(p, QString::number(tab.unread), itemX + 5, (tabsHeight - 16) / 2, unreadSt);
			}

			currentX += tabWidth + 6;
		}
		p.restore();

		p.save();
		p.setClipRect(0, tabsHeight, width(), height() - tabsHeight);
		for (auto row = 0; row < 4; ++row) {
			const auto rowY = 46 + row * 40;
			p.setPen(Qt::NoPen);
			p.setBrush(st::rndPreviewFill);
			p.drawEllipse(12, rowY, 32, 32);
			p.setBrush(st::rndSkeletonFill);
			const auto nameW = (row == 0) ? 80 : (row == 1) ? 100 : (row == 2) ? 70 : 90;
			const auto textW = (row == 0) ? 130 : (row == 1) ? 140 : (row == 2) ? 100 : 120;
			p.drawRoundedRect(54, rowY + 5, nameW, 9, 4, 4);
			p.drawRoundedRect(54, rowY + 19, textW, 8, 4, 4);
			p.drawRoundedRect(width() - 36, rowY + 5, 24, 8, 4, 4);
		}
		p.restore();
	} else {
		const auto sidebarWidth = 72;
		p.fillRect(0, 0, sidebarWidth, height(), st::windowFiltersButton.textBg);
		p.fillRect(sidebarWidth - 1, 0, 1, height(), st::shadowFg);

		const auto showIcon = RabbitSettings::foldersShowIcon();
		const auto buttonHeight = showIcon
			? st::windowFiltersButton.minHeight
			: st::windowFiltersButtonTextOnly.minHeight;
		const auto gap = 4;
		const auto count = std::min(int(tabs.size()), 3);
		const auto totalButtonsHeight = count * buttonHeight + (count - 1) * gap;
		const auto startY = std::max((height() - totalButtonsHeight) / 2, 4);

		auto unreadSt = Ui::UnreadBadgeStyle();
		unreadSt.align = style::al_left;

		p.save();
		p.setClipRect(0, 0, sidebarWidth - 1, height());

		for (auto i = 0; i < count; ++i) {
			const auto &tab = tabs[i];
			const auto active = (i == _activeTab);
			const auto y = startY + i * (buttonHeight + gap);
			const auto buttonRect = QRect(0, y, sidebarWidth - 1, buttonHeight);

			if (showIcon) {
				const auto &icons = Ui::LookupFilterIcon(tab.icon);
				const auto icon = active ? icons.active : icons.normal;
				const auto iconX = (sidebarWidth - 1 - icon->width()) / 2;
				const auto iconY = y + 6;

				if (active) {
					p.setPen(Qt::NoPen);
					if (foldersStyle == 0) {
						p.fillRect(buttonRect, st::windowFiltersButton.textBgActive);
					} else if (foldersStyle == 1) {
						p.setBrush(st::windowFiltersButton.textFgActive);
						p.drawRoundedRect(sidebarWidth - 4, y + 12, 3, buttonHeight - 24, 1.5, 1.5);
					} else if (foldersStyle == 2 || foldersStyle == 3) {
						const auto bh = 36;
						const auto cy = iconY + icon->height() / 2;
						const auto by = cy - bh / 2;
						const auto bx = 6;
						const auto bw = sidebarWidth - 1 - 12;
						const auto radius = (foldersStyle == 2) ? bh * 0.25 : bh * 0.5;
						auto col = st::windowFiltersButton.textBgActive->c;
						col.setAlpha(90);
						p.setBrush(col);
						p.drawRoundedRect(bx, by, bw, bh, radius, radius);
					}
				}

				icon->paint(p, iconX, iconY, width());

				const auto font = st::windowFiltersButton.style.font;
				p.setFont(font);
				p.setPen(active ? st::windowFiltersButton.textFgActive : st::windowFiltersButton.textFg);
				const auto maxTextWidth = sidebarWidth - 1 - 12;
				const auto elided = font->elided(tab.name, maxTextWidth);
				const auto textWidth = font->width(elided);
				const auto textX = (sidebarWidth - 1 - textWidth) / 2;
				p.drawText(textX, y + 40 + font->ascent, elided);

				if (shouldShowBadge(tab)) {
					unreadSt.muted = tab.muted;
					const auto badgeWidth = Ui::CountUnreadBadgeSize(QString::number(tab.unread), unreadSt).width();
					const auto badgeX = std::min(
						(sidebarWidth - 1) / 2 + 3,
						sidebarWidth - 1 - badgeWidth - 2);
					const auto badgeY = y + 7;
					Ui::PaintUnreadBadge(p, QString::number(tab.unread), badgeX, badgeY, unreadSt);
				}
			} else {
				const auto cy = y + buttonHeight / 2;
				if (active) {
					p.setPen(Qt::NoPen);
					if (foldersStyle == 0) {
						p.fillRect(buttonRect, st::windowFiltersButton.textBgActive);
					} else if (foldersStyle == 1) {
						p.setBrush(st::windowFiltersButton.textFgActive);
						p.drawRoundedRect(sidebarWidth - 4, y + 8, 3, buttonHeight - 16, 1.5, 1.5);
					} else if (foldersStyle == 2 || foldersStyle == 3) {
						const auto bh = 28;
						const auto by = cy - bh / 2;
						const auto bx = 6;
						const auto bw = sidebarWidth - 1 - 12;
						const auto radius = (foldersStyle == 2) ? bh * 0.25 : bh * 0.5;
						auto col = st::windowFiltersButton.textBgActive->c;
						col.setAlpha(90);
						p.setBrush(col);
						p.drawRoundedRect(bx, by, bw, bh, radius, radius);
					}
				}

				const auto font = st::windowFiltersButtonTextOnly.style.font;
				p.setFont(font);
				p.setPen(active ? st::windowFiltersButton.textFgActive : st::windowFiltersButton.textFg);
				const auto maxTextWidth = sidebarWidth - 1 - 12;
				const auto elided = font->elided(tab.name, maxTextWidth);
				const auto textWidth = font->width(elided);
				const auto textX = (sidebarWidth - 1 - textWidth) / 2;
				p.drawText(textX, y + 15 + font->ascent, elided);

				if (shouldShowBadge(tab)) {
					unreadSt.muted = tab.muted;
					const auto badgeWidth = Ui::CountUnreadBadgeSize(QString::number(tab.unread), unreadSt).width();
					const auto badgeX = std::min(
						(sidebarWidth - 1) / 2 + 15,
						sidebarWidth - 1 - badgeWidth - 2);
					const auto badgeY = y + 1;
					Ui::PaintUnreadBadge(p, QString::number(tab.unread), badgeX, badgeY, unreadSt);
				}
			}
		}
		p.restore();

		p.save();
		p.setClipRect(sidebarWidth, 0, width() - sidebarWidth, height());
		for (auto row = 0; row < 4; ++row) {
			const auto rowY = 14 + row * 48;
			p.setPen(Qt::NoPen);
			p.setBrush(st::rndPreviewFill);
			p.drawEllipse(sidebarWidth + 12, rowY, 34, 34);
			p.setBrush(st::rndSkeletonFill);
			const auto nameW = (row == 0) ? 85 : (row == 1) ? 100 : (row == 2) ? 70 : 90;
			const auto textW = (row == 0) ? 120 : (row == 1) ? 140 : (row == 2) ? 100 : 130;
			p.drawRoundedRect(sidebarWidth + 56, rowY + 5, nameW, 9, 4, 4);
			p.drawRoundedRect(sidebarWidth + 56, rowY + 20, textW, 8, 4, 4);
			p.drawRoundedRect(width() - 36, rowY + 5, 24, 8, 4, 4);
		}
		p.restore();
	}

	p.setClipping(false);
	p.setPen(QPen(st::quickActionBoxStroke, 1.));
	p.setBrush(Qt::NoBrush);
	p.drawRoundedRect(cardRect.adjusted(0, 0, -1, -1), radius, radius);
}
