/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "info/profile/tabs/info_profile_tab_content.h"

namespace Main {
class Session;
} // namespace Main

namespace Info::Profile {

[[nodiscard]] rpl::producer<TextWithEntities> SavedChatsCountStatus(
	not_null<Main::Session*> session);

[[nodiscard]] MediaTabDescriptor MakeChatsTabDescriptor();

} // namespace Info::Profile
