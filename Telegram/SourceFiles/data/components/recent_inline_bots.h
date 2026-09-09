/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class UserData;

namespace Main {
class Session;
} // namespace Main

namespace Data {

class RecentInlineBots final {
public:
	explicit RecentInlineBots(not_null<Main::Session*> session);

	[[nodiscard]] const std::vector<not_null<UserData*>> &list() const;
	[[nodiscard]] rpl::producer<> updates() const;

	void bump(not_null<UserData*> user);
	void remove(not_null<UserData*> user);
	void applyLocal(std::vector<not_null<UserData*>> list);

private:
	const not_null<Main::Session*> _session;
	std::vector<not_null<UserData*>> _list;
	rpl::event_stream<> _updates;

};

} // namespace Data
