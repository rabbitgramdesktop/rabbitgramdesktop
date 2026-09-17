/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "rabbit/quick_actions/quick_action.h"
#include "rabbit/swipe_actions/swipe_action.h"
#include "base/basic_types.h"

class HistoryItem;

namespace HistoryView {
class Element;
} // namespace HistoryView

namespace RabbitUtils::Chats {

[[nodiscard]] bool isSwipeActionAllowed(
	RabbitSettings::SwipeAction action,
	not_null<HistoryItem*> item,
	HistoryView::Element *view,
	bool hasCopyRestr,
	bool hasCopyMediaRestr);

[[nodiscard]] bool isQuickActionAllowed(
	RabbitSettings::QuickAction action,
	not_null<HistoryItem*> item,
	HistoryView::Element *view,
	bool hasCopyRestr,
	bool hasCopyMediaRestr);

} // namespace RabbitUtils::Chats
