/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {

// Values are serialized in Core::Settings, don't change them.
enum class ChatsFiltersTabsMode {
	Default = 0,
	TextAndIcons = 1,
	IconsOnly = 2,
	TextOnly = 3,
};

[[nodiscard]] inline ChatsFiltersTabsMode HorizontalChatsFiltersTabsMode(
		ChatsFiltersTabsMode mode) {
	return (mode == ChatsFiltersTabsMode::Default)
		? ChatsFiltersTabsMode::TextOnly
		: mode;
}

[[nodiscard]] inline ChatsFiltersTabsMode VerticalChatsFiltersTabsMode(
		ChatsFiltersTabsMode mode) {
	return (mode == ChatsFiltersTabsMode::Default)
		? ChatsFiltersTabsMode::TextAndIcons
		: mode;
}

} // namespace Ui
