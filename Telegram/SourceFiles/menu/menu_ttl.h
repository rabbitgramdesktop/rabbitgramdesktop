/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {
class Show;
class GenericBox;
} // namespace Ui

namespace TTLMenu {

struct Args {
	std::shared_ptr<Ui::Show> show;
	TimeId startTtl;
	rpl::producer<TextWithEntities> about;
	Fn<void(TimeId, Fn<void()>)> callback;
	bool hideDisable = false;
};

void TTLBox(not_null<Ui::GenericBox*> box, Args args);

} // namespace TTLMenu
