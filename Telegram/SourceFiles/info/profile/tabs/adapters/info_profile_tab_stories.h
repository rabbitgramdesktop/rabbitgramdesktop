/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "info/profile/tabs/info_profile_tab_content.h"

namespace Info::Profile {

[[nodiscard]] MediaTabDescriptor MakeStoriesTabDescriptor(
	not_null<PeerData*> peer);

} // namespace Info::Profile
