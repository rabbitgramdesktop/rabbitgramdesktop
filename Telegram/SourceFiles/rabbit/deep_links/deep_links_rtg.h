/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Core::DeepLinks {

class Router;

void RegisterRtgHandlers(Router &router);

[[nodiscard]] QString RtgSettingsDeepLink(const QString &controlId);

} // namespace Core::DeepLinks
