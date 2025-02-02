/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {
struct GiveawayStart;
struct GiveawayResults;
} // namespace Data

namespace HistoryView {

class Element;
class MediaGenericPart;

[[nodiscard]] auto GenerateGiveawayStart(
	not_null<Element*> parent,
	not_null<Data::GiveawayStart*> data)
-> Fn<void(Fn<void(std::unique_ptr<MediaGenericPart>)>)>;

[[nodiscard]] auto GenerateGiveawayResults(
	not_null<Element*> parent,
	not_null<Data::GiveawayResults*> data)
-> Fn<void(Fn<void(std::unique_ptr<MediaGenericPart>)>)>;

} // namespace HistoryView
