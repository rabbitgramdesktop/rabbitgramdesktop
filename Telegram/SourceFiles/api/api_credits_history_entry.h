/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class PeerData;

namespace Data {
struct CreditsHistoryEntry;
} // namespace Data

namespace Api {

[[nodiscard]] Data::CreditsHistoryEntry CreditsHistoryEntryFromTL(
	const MTPStarsTransaction &tl,
	not_null<PeerData*> peer);

} // namespace Api
