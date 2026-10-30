/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class PeerData;

namespace Info::Saved {

struct MusicTag {
	explicit MusicTag(not_null<PeerData*> peer)
	: peer(peer) {
	}

	not_null<PeerData*> peer;
};

} // namespace Info::Saved
