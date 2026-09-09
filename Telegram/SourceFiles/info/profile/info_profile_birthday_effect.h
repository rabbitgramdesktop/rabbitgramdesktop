/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class UserData;

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Info::Profile {

void StartProfileBirthdayEffect(
	not_null<Ui::RpWidget*> cover,
	not_null<UserData*> user,
	Fn<QRect()> userpicGeometry,
	Fn<bool()> paused);

} // namespace Info::Profile
