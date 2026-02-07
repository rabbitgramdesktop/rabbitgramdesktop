/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "settings/settings_common.h"
#include "ui/layers/generic_box.h"

not_null<Settings::Button*> AddQuickActionButton(
    not_null<Ui::VerticalLayout*> container,
    rpl::producer<QString> text,
    const style::SettingsButton &st,
    Settings::IconDescriptor &&descriptor);
void QuickActionBox(not_null<Ui::GenericBox*> box);