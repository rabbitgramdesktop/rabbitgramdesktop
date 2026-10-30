/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Dialogs {
struct UnreadState;
} // namespace Dialogs

namespace Main {
class Session;
} // namespace Main

namespace Data {

[[nodiscard]] Dialogs::UnreadState MainListMapUnreadState(
	not_null<Main::Session*> session,
	const Dialogs::UnreadState &state);

[[nodiscard]] rpl::producer<Dialogs::UnreadState> UnreadStateValue(
	not_null<Main::Session*> session,
	FilterId filterId);

[[nodiscard]] rpl::producer<bool> IncludeMutedCounterFoldersValue();

[[nodiscard]] std::optional<int> ComputeUnreadCountLocally(
	MsgId afterId,
	MsgId minReadTillId,
	const std::vector<MsgId> &list,
	std::optional<int> skippedBefore,
	std::optional<int> skippedAfter,
	std::optional<int> fullCount,
	std::optional<int> currentUnreadCountAfter,
	MsgId wasReadTillId);
[[nodiscard]] int ReconcileSkippedCounts(
	std::optional<int> &skippedBefore,
	std::optional<int> &skippedAfter,
	bool toFront,
	int wasSize,
	int nowSize,
	int fullCount);

} // namespace Data
