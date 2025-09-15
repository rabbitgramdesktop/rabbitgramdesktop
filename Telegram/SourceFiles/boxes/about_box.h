/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/layers/generic_box.h"

void AboutBox(not_null<Ui::GenericBox*> box);
void ArchiveHintBox(
	not_null<Ui::GenericBox*> box,
	bool unarchiveOnNewMessage,
	Fn<void()> onUnarchive);

QString telegramFaqLink();
QString currentVersionText();
