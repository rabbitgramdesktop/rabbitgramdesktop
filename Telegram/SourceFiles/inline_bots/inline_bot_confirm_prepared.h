/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {
class Thread;
} // namespace Data

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class GenericBox;
} // namespace Ui

namespace InlineBots {

void PreparedPreviewBox(
	not_null<Ui::GenericBox*> box,
	not_null<HistoryItem*> item,
	rpl::producer<not_null<Data::Thread*>> recipient,
	Fn<void()> choose,
	Fn<void(not_null<Data::Thread*>)> sent);

} // namespace InlineBots
