/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/utils/chat_utils.h"

#include "rabbit/settings/rabbit_settings.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/history_inner_widget.h"
#include "history/view/history_view_element.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "main/main_session.h"
#include "data/data_session.h"
#include "data/data_message_reactions.h"

namespace RabbitUtils::Chats {

bool isSwipeActionAllowed(
		RabbitSettings::SwipeAction action,
		not_null<HistoryItem*> item,
		HistoryView::Element *view,
		bool hasCopyRestr,
		bool hasCopyMediaRestr) {
	switch (action) {
	case RabbitSettings::SwipeAction::Disable:
		return false;
	case RabbitSettings::SwipeAction::Reply:
		return CanSendReply(item);
	case RabbitSettings::SwipeAction::Reaction:
		return !item->isService() && ranges::contains(
			Data::LookupPossibleReactions(item).recent,
			item->history()->owner().session().data().reactions().favoriteId(),
			[](not_null<const Data::Reaction*> r) { return r->id; });
	case RabbitSettings::SwipeAction::Copy:
		if (!hasCopyRestr && (!item->originalText().text.isEmpty() || (view && view->hasVisibleText()))) {
			return true;
		}
		if (!hasCopyMediaRestr && item->media() != nullptr) {
			return true;
		}
		return false;
	case RabbitSettings::SwipeAction::Forward:
		return !hasCopyRestr && item->allowsForward();
	case RabbitSettings::SwipeAction::Edit:
		return item->allowsEdit(crl::now());
	case RabbitSettings::SwipeAction::Save:
		return !hasCopyRestr && item->allowsForward();
	case RabbitSettings::SwipeAction::Repeat:
		return !hasCopyRestr && item->allowsForward();
	case RabbitSettings::SwipeAction::Delete:
		return item->canDelete();
	case RabbitSettings::SwipeAction::Translate:
		return !item->originalText().text.isEmpty() && Core::App().settings().translateButtonEnabled();
	}
	return false;
}

bool isQuickActionAllowed(
		RabbitSettings::QuickAction action,
		not_null<HistoryItem*> item,
		HistoryView::Element *view,
		bool hasCopyRestr,
		bool hasCopyMediaRestr) {
	return isSwipeActionAllowed(
		static_cast<RabbitSettings::SwipeAction>(static_cast<int>(action)),
		item,
		view,
		hasCopyRestr,
		hasCopyMediaRestr);
}

} // namespace RabbitUtils::Chats