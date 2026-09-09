/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "webview/webview_interface.h"

namespace Core {

[[nodiscard]] inline const Webview::Available &CachedWebviewAvailability() {
	static const auto result = Webview::Availability();
	return result;
}

} // namespace Core
