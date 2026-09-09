/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class History;

namespace Main {
class Session;
} // namespace Main

namespace Data {

struct MessagesSlice;
struct MessagePosition;

[[nodiscard]] rpl::producer<MessagesSlice> PollMessagesViewer(
	not_null<Main::Session*> session,
	not_null<History*> history,
	MsgId topicRootId,
	PeerId monoforumPeerId,
	MessagePosition aroundId,
	int limitBefore,
	int limitAfter);

} // namespace Data
