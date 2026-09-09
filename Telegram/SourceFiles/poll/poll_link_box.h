/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {
class GenericBox;
} // namespace Ui

namespace Poll {

void AddPollOptionLinkBox(
	not_null<Ui::GenericBox*> box,
	const QString &initial,
	Fn<void(QString)> callback);

} // namespace Poll
