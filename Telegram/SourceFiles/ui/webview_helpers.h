/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/flat_map.h"

namespace tr {
template <typename ...Tags>
struct phrase;
} // namespace tr

namespace Ui {

[[nodiscard]] QByteArray ComputeStyles(
	const base::flat_map<QByteArray, const style::color*> &colors,
	const base::flat_map<QByteArray, tr::phrase<>> &phrases,
	int zoom,
	bool nightTheme = false);
[[nodiscard]] QByteArray ComputeSemiTransparentOverStyle(
	const QByteArray &name,
	const style::color &over,
	const style::color &bg);

[[nodiscard]] QByteArray EscapeForAttribute(QByteArray value);
[[nodiscard]] QByteArray EscapeForScriptString(QByteArray value);

} // namespace Ui
