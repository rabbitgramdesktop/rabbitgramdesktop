/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/layers/generic_box.h"

namespace Window {
class SessionController;
} // namespace Window

void AppIconBox(
	not_null<Ui::GenericBox*> box,
	Window::SessionController *controller = nullptr,
	const QString &selectedId = QString());

void SetPendingReopenAppIconId(const QString &id);
[[nodiscard]] QString TakePendingReopenAppIconId();
