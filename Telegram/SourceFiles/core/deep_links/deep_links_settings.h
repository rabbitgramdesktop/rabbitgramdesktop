/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "settings/settings_type.h"

namespace Core::DeepLinks {

class Router;

void RegisterSettingsHandlers(Router &router);

[[nodiscard]] QString SettingsDeepLink(
	::Settings::Type section,
	const QString &controlId);

} // namespace Core::DeepLinks
