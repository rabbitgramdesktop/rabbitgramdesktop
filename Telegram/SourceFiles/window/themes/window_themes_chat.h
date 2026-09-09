/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "data/data_cloud_themes.h"

namespace Ui {
struct ChatThemeBubblesData;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

namespace Window::Theme {

struct Preview;

[[nodiscard]] std::optional<Data::CloudThemeType> ChatThemeVariant(
	const Data::CloudTheme &theme,
	bool dark);

[[nodiscard]] Ui::ChatThemeBubblesData PrepareBubblesData(
	const Data::CloudTheme &theme,
	Data::CloudThemeType type);

[[nodiscard]] std::unique_ptr<Preview> PreviewFromChatTheme(
	const Data::CloudTheme &theme,
	bool dark);

[[nodiscard]] bool ChatThemeOwnsPaper(const Data::CloudTheme &theme);

void ApplyChatTheme(
	not_null<SessionController*> controller,
	const Data::CloudTheme &theme,
	bool dark,
	bool replacePaper = true);

void CheckChatThemeWallPaper(not_null<SessionController*> controller);

} // namespace Window::Theme
