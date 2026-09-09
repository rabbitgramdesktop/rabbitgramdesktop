/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "boxes/peers/community_pending_requests_box.h"

#include "data/data_channel.h"
#include "info/info_controller.h"
#include "info/info_memento.h"
#include "window/window_session_controller.h"

void ShowCommunityPendingRequestsBox(
		not_null<Window::SessionNavigation*> navigation,
		not_null<ChannelData*> community) {
	navigation->showSection(
		std::make_shared<Info::Memento>(
			community,
			Info::Section::Type::CommunityRequests));
}
