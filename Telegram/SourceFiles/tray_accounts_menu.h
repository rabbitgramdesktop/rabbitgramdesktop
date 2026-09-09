/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "platform/platform_tray.h"

namespace Core::TrayAccountsMenu {

void SetupChangesSubscription(
	Fn<void()> callback,
	rpl::lifetime &lifetime);
void Fill(Platform::Tray &tray);

} // namespace Core::TrayAccountsMenu
