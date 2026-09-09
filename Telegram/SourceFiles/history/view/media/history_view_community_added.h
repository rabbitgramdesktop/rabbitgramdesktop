/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class ChannelData;

namespace HistoryView {

class Element;
class MediaGeneric;
class MediaGenericPart;

[[nodiscard]] auto GenerateCommunityAddedMedia(
	not_null<Element*> parent,
	not_null<ChannelData*> community
) -> Fn<void(
	not_null<MediaGeneric*>,
	Fn<void(std::unique_ptr<MediaGenericPart>)>)>;

} // namespace HistoryView
