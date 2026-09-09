/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QSize>
#include <QtGui/QImage>

namespace Ui {

[[nodiscard]] int SvgPreviewBytesLimit();
[[nodiscard]] QImage RenderSvgPreview(
	const QByteArray &bytes,
	QSize maxSize);

} // namespace Ui
