/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "platform/platform_text_recognition.h"

namespace Platform {
namespace TextRecognition {

inline bool IsAvailable() {
	return false;
}

inline Result RecognizeText(const QImage &image) {
	return {};
}

} // namespace TextRecognition
} // namespace Platform