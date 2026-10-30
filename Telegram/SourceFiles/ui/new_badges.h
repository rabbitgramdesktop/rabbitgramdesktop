/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/text/custom_emoji_helper.h"

namespace style {
struct SettingsButton;
} // namespace style

namespace Ui {

class RpWidget;

[[nodiscard]] Text::PaletteDependentEmoji AttentionMarkEmoji();

} // namespace Ui

namespace Ui::NewBadge {

[[nodiscard]] not_null<Ui::RpWidget*> CreateNewBadge(
	not_null<Ui::RpWidget*> parent,
	rpl::producer<QString> text);

void AddToRight(not_null<Ui::RpWidget*> parent);
void AddAfterLabel(
	not_null<Ui::RpWidget*> parent,
	not_null<Ui::RpWidget*> label);
void AddAfterButtonText(
	not_null<Ui::RpWidget*> button,
	rpl::producer<QString> text,
	const style::SettingsButton &st);

} // namespace Ui::NewBadge
