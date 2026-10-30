/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "settings/settings_common.h"
#include "settings/settings_type.h"

enum class PremiumFeature;

namespace Main {
class Session;
} // namespace Main

namespace Settings {

[[nodiscard]] Type BusinessId();

[[nodiscard]] std::vector<PremiumFeature> BusinessFeaturesOrder(
	not_null<::Main::Session*> session);

} // namespace Settings
