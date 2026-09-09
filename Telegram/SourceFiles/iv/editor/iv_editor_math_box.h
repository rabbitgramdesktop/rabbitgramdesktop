/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/basic_types.h"

#include <QtCore/QString>

#include <optional>

namespace Ui {
class GenericBox;
} // namespace Ui

namespace Iv::Editor {

void EditMathBox(
	not_null<Ui::GenericBox*> box,
	QString startSource,
	bool editingExisting,
	std::optional<bool> separateLine,
	Fn<void(QString, bool)> callback,
	Fn<void(bool)> setExternalInteractionActive,
	Fn<void()> restoreFocus);

} // namespace Iv::Editor
