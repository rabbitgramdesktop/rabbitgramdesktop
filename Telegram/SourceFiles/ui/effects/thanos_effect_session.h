/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Main {
class Session;
} // namespace Main

namespace Ui {

void ScheduleThanosEffectWarmUp(
	not_null<Main::Session*> session,
	rpl::lifetime &lifetime);

} // namespace Ui
