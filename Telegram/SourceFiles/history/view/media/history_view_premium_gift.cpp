/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "history/view/media/history_view_premium_gift.h"

#include "base/unixtime.h"
#include "boxes/gift_premium_box.h" // ResolveGiftCode
#include "chat_helpers/stickers_gift_box_pack.h"
#include "core/click_handler_types.h" // ClickHandlerContext
#include "data/data_channel.h"
#include "data/data_credits.h"
#include "data/data_document.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/settings_credits.h" // Settings::CreditsId
#include "settings/settings_credits_graphics.h"
#include "settings/settings_credits_graphics.h" // GiftedCreditsBox
#include "settings/settings_premium.h" // Settings::ShowGiftPremium
#include "ui/layers/generic_box.h"
#include "ui/text/text_utilities.h"
#include "window/window_session_controller.h"
#include "styles/style_chat.h"

namespace HistoryView {

PremiumGift::PremiumGift(
	not_null<Element*> parent,
	not_null<Data::MediaGiftBox*> gift)
: _parent(parent)
, _gift(gift)
, _data(gift->data()) {
}

PremiumGift::~PremiumGift() = default;

int PremiumGift::top() {
	return st::msgServiceGiftBoxStickerTop;
}

QSize PremiumGift::size() {
	return QSize(
		st::msgServiceGiftBoxStickerSize,
		st::msgServiceGiftBoxStickerSize);
}

QString PremiumGift::title() {
	if (creditsPrize()) {
		return tr::lng_prize_title(tr::now);
	} else if (const auto count = credits()) {
		return tr::lng_gift_stars_title(tr::now, lt_count, count);
	}
	return gift()
		? tr::lng_premium_summary_title(tr::now)
		: _data.unclaimed
		? tr::lng_prize_unclaimed_title(tr::now)
		: tr::lng_prize_title(tr::now);
}

TextWithEntities PremiumGift::subtitle() {
	const auto isCreditsPrize = creditsPrize();
	if (const auto count = credits(); count && !isCreditsPrize) {
		return outgoingGift()
			? tr::lng_gift_stars_outgoing(
				tr::now,
				lt_user,
				Ui::Text::Bold(_parent->history()->peer->shortName()),
				Ui::Text::WithEntities)
			: tr::lng_gift_stars_incoming(tr::now, Ui::Text::WithEntities);
	} else if (gift()) {
		return { GiftDuration(_data.count) };
	}
	const auto name = _data.channel ? _data.channel->name() : "channel";
	auto result = (_data.unclaimed
		? tr::lng_prize_unclaimed_about
		: _data.viaGiveaway
		? tr::lng_prize_about
		: tr::lng_prize_gift_about)(
			tr::now,
			lt_channel,
			Ui::Text::Bold(name),
			Ui::Text::RichLangValue);
	result.append("\n\n");
	result.append(isCreditsPrize
		? tr::lng_prize_credits(
			tr::now,
			lt_amount,
			tr::lng_prize_credits_amount(
				tr::now,
				lt_count,
				credits(),
				Ui::Text::RichLangValue),
			Ui::Text::RichLangValue)
		: (_data.unclaimed
			? tr::lng_prize_unclaimed_duration
			: _data.viaGiveaway
			? tr::lng_prize_duration
			: tr::lng_prize_gift_duration)(
				tr::now,
				lt_duration,
				Ui::Text::Bold(GiftDuration(_data.count)),
				Ui::Text::RichLangValue));
	return result;
}

rpl::producer<QString> PremiumGift::button() {
	return creditsPrize()
		? tr::lng_view_button_giftcode()
		: (gift() && (outgoingGift() || !_data.unclaimed))
		? tr::lng_sticker_premium_view()
		: tr::lng_prize_open();
}

ClickHandlerPtr PremiumGift::createViewLink() {
	const auto from = _gift->from();
	const auto peer = _parent->history()->peer;
	const auto date = _parent->data()->date();
	const auto data = _gift->data();
	return std::make_shared<LambdaClickHandler>([=](ClickContext context) {
		const auto my = context.other.value<ClickHandlerContext>();
		if (const auto controller = my.sessionWindow.get()) {
			const auto selfId = controller->session().userPeerId();
			const auto sent = (from->id == selfId);
			if (creditsPrize()) {
				using Type = Data::CreditsHistoryEntry::PeerType;
				controller->show(Box(
					Settings::ReceiptCreditsBox,
					controller,
					Data::CreditsHistoryEntry{
						.id = data.slug,
						.title = QString(),
						.description = QString(),
						.date = base::unixtime::parse(date),
						.credits = uint64(data.count),
						.barePeerId = data.channel
							? data.channel->id.value
							: 0,
						.bareGiveawayMsgId = uint64(data.giveawayMsgId),
						.peerType = Type::Peer,
						.in = true,
					},
					Data::SubscriptionEntry()));
			} else if (data.type == Data::GiftType::Credits) {
				const auto to = sent ? peer : peer->session().user();
				controller->show(Box(
					Settings::GiftedCreditsBox,
					controller,
					from,
					to,
					data.count,
					date));
			} else if (data.slug.isEmpty()) {
				const auto months = data.count;
				Settings::ShowGiftPremium(controller, peer, months, sent);
			} else {
				const auto fromId = from->id;
				const auto toId = sent ? peer->id : selfId;
				ResolveGiftCode(controller, data.slug, fromId, toId);
			}
		}
	});
}

int PremiumGift::buttonSkip() {
	return st::msgServiceGiftBoxButtonMargins.top();
}

void PremiumGift::draw(
		Painter &p,
		const PaintContext &context,
		const QRect &geometry) {
	if (_sticker) {
		_sticker->draw(p, context, geometry);
	} else {
		ensureStickerCreated();
	}
}

bool PremiumGift::hideServiceText() {
	return !gift();
}

void PremiumGift::stickerClearLoopPlayed() {
	if (_sticker) {
		_sticker->stickerClearLoopPlayed();
	}
}

std::unique_ptr<StickerPlayer> PremiumGift::stickerTakePlayer(
		not_null<DocumentData*> data,
		const Lottie::ColorReplacements *replacements) {
	return _sticker
		? _sticker->stickerTakePlayer(data, replacements)
		: nullptr;
}

bool PremiumGift::hasHeavyPart() {
	return (_sticker ? _sticker->hasHeavyPart() : false);
}

void PremiumGift::unloadHeavyPart() {
	if (_sticker) {
		_sticker->unloadHeavyPart();
	}
}

bool PremiumGift::incomingGift() const {
	return gift() && !_parent->data()->out();
}

bool PremiumGift::outgoingGift() const {
	return gift() && _parent->data()->out();
}

bool PremiumGift::gift() const {
	return _data.slug.isEmpty() || !_data.channel;
}

bool PremiumGift::creditsPrize() const {
	return _data.viaGiveaway
		&& (_data.type == Data::GiftType::Credits)
		&& !_data.slug.isEmpty();
}

int PremiumGift::credits() const {
	return (_data.type == Data::GiftType::Credits) ? _data.count : 0;
}

void PremiumGift::ensureStickerCreated() const {
	if (_sticker) {
		return;
	}
	const auto &session = _parent->history()->session();
	auto &packs = session.giftBoxStickersPacks();
	const auto count = credits();
	const auto months = count ? packs.monthsForStars(count) : _data.count;
	if (const auto document = packs.lookup(months)) {
		if (const auto sticker = document->sticker()) {
			const auto skipPremiumEffect = false;
			_sticker.emplace(_parent, document, skipPremiumEffect, _parent);
			_sticker->setDiceIndex(sticker->alt, 1);
			_sticker->initSize(st::msgServiceGiftBoxStickerSize);
		}
	}
}

} // namespace HistoryView
