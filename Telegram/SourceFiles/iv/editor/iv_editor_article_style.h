/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "styles/style_iv.h"

class QTextDocument;

namespace Iv::Editor {

[[nodiscard]] style::Markdown CreateEditorMarkdownStyle();

[[nodiscard]] const style::margins &EditorBodyPadding();

[[nodiscard]] int MaxVisualLineWidth(not_null<const QTextDocument*> document);

[[nodiscard]] int MaxVisualLineWidthForWidth(
	not_null<const QTextDocument*> document,
	int width);

} // namespace Iv::Editor
