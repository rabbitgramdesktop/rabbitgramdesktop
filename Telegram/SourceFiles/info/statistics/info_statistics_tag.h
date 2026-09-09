/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class PeerData;

namespace Info::Statistics {

struct Tag final {
	explicit Tag() = default;
	explicit Tag(
		PeerData *peer,
		FullMsgId contextId,
		FullStoryId storyId)
	: peer(peer)
	, contextId(contextId)
	, storyId(storyId) {
	}

	PeerData *peer = nullptr;
	FullMsgId contextId;
	FullStoryId storyId;

};

} // namespace Info::Statistics
