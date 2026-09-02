/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <rpl/producer.h>

#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/quick_action.h"
#include "settings/settings_common.h"
#include "styles/style_menu_icons.h"

namespace RabbitSettings {

inline rpl::producer<QString> QuickActionString(QuickAction action) {
	switch (action) {
	case Disable: return rktr(u"quick_action_disable"_q);
	case Reaction: return rktr(u"quick_action_reaction"_q);
	case Reply: return rktr(u"quick_action_reply"_q);
	case Copy: return rktr(u"quick_action_copy"_q);
	case Forward: return rktr(u"quick_action_forward"_q);
	case Edit: return rktr(u"quick_action_edit"_q);
	case Save: return rktr(u"quick_action_save"_q);
	case Delete: return rktr(u"quick_action_delete"_q);
	case Repeat: return rktr(u"quick_action_repeat"_q);
	case Translate: return rktr(u"quick_action_translate"_q);
	}
	return rktr(u"quick_action_disable"_q);
}

inline Settings::IconDescriptor QuickActionIcon(QuickAction action) {
	switch (action) {
	case Disable: return { &st::menuIconBlock };
	case Reaction: return { &st::menuIconGroupReactions };
	case Reply: return { &st::menuIconReply };
	case Copy: return { &st::menuIconCopy };
	case Forward: return { &st::menuIconForward };
	case Edit: return { &st::menuIconEdit };
	case Save: return { &st::menuIconSavedMessages };
	case Delete: return { &st::menuIconDelete };
	case Repeat: return { &st::menuIconRestore };
	case Translate: return { &st::menuIconTranslate };
	}
	return {};
}

} // namespace RabbitSettings
