/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class QKeyEvent;

namespace Ui {
class InputField;
} // namespace Ui

namespace Iv::Editor {

[[nodiscard]] bool HandleAutoPairKey(
	not_null<Ui::InputField*> field,
	not_null<QKeyEvent*> e);

} // namespace Iv::Editor
