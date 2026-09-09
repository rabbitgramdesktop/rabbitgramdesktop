/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/weak_ptr.h"

namespace Data {
struct AiComposeTone;
} // namespace Data

namespace Main {
class Session;
} // namespace Main

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {
class GenericBox;
} // namespace Ui

void PreviewAiToneBox(
	not_null<Ui::GenericBox*> box,
	not_null<Main::Session*> session,
	Data::AiComposeTone tone,
	base::weak_ptr<Window::SessionController> controller);
