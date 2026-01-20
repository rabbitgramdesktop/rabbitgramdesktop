/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class PeerData;

namespace Data {
class SavedSublist;
} // namespace Data

namespace Ui {
class GenericBox;
} // namespace Ui

extern const char kModerateCommonGroups[];

void CreateModerateMessagesBox(
	not_null<Ui::GenericBox*> box,
	const HistoryItemsList &items,
	Fn<void()> confirmed);

[[nodiscard]] bool CanCreateModerateMessagesBox(const HistoryItemsList &);

void DeleteChatBox(not_null<Ui::GenericBox*> box, not_null<PeerData*> peer);
void DeleteSublistBox(
	not_null<Ui::GenericBox*> box,
	not_null<Data::SavedSublist*> sublist);
