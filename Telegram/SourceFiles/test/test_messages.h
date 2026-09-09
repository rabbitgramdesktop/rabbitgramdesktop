/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/basic_types.h"

struct MsgId;
class History;
class HistoryItem;

namespace rpl {
class lifetime;
} // namespace rpl

namespace Test {

class SentMessageWatcher final {
public:
	using Predicate = Fn<bool(not_null<HistoryItem*>)>;

	// Subscribes immediately; the caller lifetime owns both subscriptions.
	SentMessageWatcher(
		not_null<History*> history,
		MsgId highWater,
		Predicate predicate,
		QString diagnosticTag,
		rpl::lifetime &lifetime);

	// Returns the matching server item, or null with a throttled probe.
	[[nodiscard]] HistoryItem *poll();

private:
	struct State;
	const std::shared_ptr<State> _state;

};

} // namespace Test
