/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class UserData;

namespace Ui {
class PopupMenu;
} // namespace Ui

namespace Info {
namespace Profile {

[[nodiscard]] bool IsCollectiblePhone(not_null<UserData*> user);

void AddPhoneMenu(not_null<Ui::PopupMenu*> menu, not_null<UserData*> user);
void AddPhoneSpoilerMenu(
	not_null<Ui::PopupMenu*> menu,
	not_null<UserData*> user);

} // namespace Profile
} // namespace Info
