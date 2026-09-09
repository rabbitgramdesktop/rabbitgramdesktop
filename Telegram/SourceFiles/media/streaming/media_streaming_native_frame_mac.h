/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#ifdef Q_OS_MAC

#include <QtGui/QImage>

namespace Media::Streaming {

struct NativeFrame;

[[nodiscard]] QImage ConvertNativeFrameToARGB32(const NativeFrame &frame);

} // namespace Media::Streaming

#endif // Q_OS_MAC
