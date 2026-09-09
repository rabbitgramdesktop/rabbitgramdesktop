/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/editor/iv_editor_state.h"

namespace Iv::Editor {

struct CommandContext {
	not_null<State*> state;
	State::ActiveEnterContext enter;
	bool caretAtStart = false;
	int targetOrdinal = -1;
};

[[nodiscard]] bool RunEnterChain(CommandContext &context);

} // namespace Iv::Editor
