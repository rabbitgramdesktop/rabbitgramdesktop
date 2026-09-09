/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class HistoryItem;

namespace Menu {

[[nodiscard]] Fn<void(bool)> RateTranscribeCallbackFactory(
	not_null<HistoryItem*>);

[[nodiscard]] bool HasRateTranscribeItem(not_null<HistoryItem*>);

} // namespace Menu
