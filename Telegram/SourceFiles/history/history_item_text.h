/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class HistoryItem;

namespace Data {
struct Group;
} // namespace Data

TextForMimeData HistoryItemText(not_null<HistoryItem*> item);
TextForMimeData HistoryGroupText(not_null<const Data::Group*> group);
TextForMimeData HistoryItemTextForSelectedCopy(not_null<HistoryItem*> item);
TextForMimeData HistoryGroupTextForSelectedCopy(
	not_null<const Data::Group*> group);
TextForMimeData HistorySelectedItemWrappedText(
	not_null<HistoryItem*> item,
	TextForMimeData &&body,
	bool richContext);
