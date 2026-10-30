/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/iv_rich_page.h"

class HistoryItem;
class PhotoData;

namespace Data {
struct Group;
} // namespace Data

struct HistorySelectedTextEntry {
	not_null<HistoryItem*> item;
	const Data::Group *group = nullptr;
};

struct HistorySelectedTextImage {
	not_null<PhotoData*> photo;
	QString before;
	QString after;
};

struct HistorySelectedTextParts {
	std::vector<TextForMimeData> texts;
	std::vector<HistorySelectedTextImage> images;

	[[nodiscard]] bool empty() const;
	[[nodiscard]] TextForMimeData composed(
		const base::flat_map<not_null<PhotoData*>, QString> &blocks = {}) const;
};

TextForMimeData HistoryItemText(not_null<HistoryItem*> item);
[[nodiscard]] Iv::RichPageBlocksSlice HistoryItemRichBlocks(
	not_null<HistoryItem*> item);
TextForMimeData HistoryGroupText(not_null<const Data::Group*> group);
[[nodiscard]] HistorySelectedTextParts HistorySelectedItemsTextParts(
	const std::vector<HistorySelectedTextEntry> &entries,
	bool richContext);
