/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class ChannelData;
class PeerData;

namespace Window {
class SessionNavigation;
} // namespace Window

void ShowAddToCommunityBox(
	not_null<Window::SessionNavigation*> navigation,
	not_null<PeerData*> peer);

void ShowAddPeerToCommunity(
	not_null<Window::SessionNavigation*> navigation,
	not_null<ChannelData*> community,
	not_null<PeerData*> peer,
	Fn<void()> completed = nullptr);
