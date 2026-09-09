/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <rpl/producer.h>

#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/swipe_actions/swipe_action.h"
#include "settings/settings_common.h"
#include "styles/style_menu_icons.h"

namespace RabbitSettings {

inline rpl::producer<QString> SwipeActionString(SwipeAction action) {
	switch (action) {
	case SwipeAction::Disable: return rktr(u"quick_action_disable"_q);
	case SwipeAction::Reaction: return rktr(u"quick_action_reaction"_q);
	case SwipeAction::Reply: return rktr(u"quick_action_reply"_q);
	case SwipeAction::Copy: return rktr(u"quick_action_copy"_q);
	case SwipeAction::Forward: return rktr(u"quick_action_forward"_q);
	case SwipeAction::Edit: return rktr(u"quick_action_edit"_q);
	case SwipeAction::Save: return rktr(u"quick_action_save"_q);
	case SwipeAction::Delete: return rktr(u"quick_action_delete"_q);
	case SwipeAction::Repeat: return rktr(u"quick_action_repeat"_q);
	case SwipeAction::Translate: return rktr(u"quick_action_translate"_q);
	}
	return rktr(u"quick_action_disable"_q);
}

inline Settings::IconDescriptor SwipeActionIcon(SwipeAction action) {
	switch (action) {
	case SwipeAction::Disable: return { &st::menuIconBlock };
	case SwipeAction::Reaction: return { &st::menuIconGroupReactions };
	case SwipeAction::Reply: return { &st::menuIconReply };
	case SwipeAction::Copy: return { &st::menuIconCopy };
	case SwipeAction::Forward: return { &st::menuIconForward };
	case SwipeAction::Edit: return { &st::menuIconEdit };
	case SwipeAction::Save: return { &st::menuIconSavedMessages };
	case SwipeAction::Delete: return { &st::menuIconDelete };
	case SwipeAction::Repeat: return { &st::menuIconRestore };
	case SwipeAction::Translate: return { &st::menuIconTranslate };
	}
	return {};
}

} // namespace RabbitSettings
