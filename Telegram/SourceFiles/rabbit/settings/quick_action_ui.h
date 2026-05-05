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
	case Disable: return rktr("quick_action_disable");
	case Reaction: return rktr("quick_action_reaction");
	case Reply: return rktr("quick_action_reply");
	case Copy: return rktr("quick_action_copy");
	case Forward: return rktr("quick_action_forward");
	case Edit: return rktr("quick_action_edit");
	case Save: return rktr("quick_action_save");
	case Delete: return rktr("quick_action_delete");
	}
	return rktr("quick_action_disable");
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
	}
	return {};
}

} // namespace RabbitSettings
