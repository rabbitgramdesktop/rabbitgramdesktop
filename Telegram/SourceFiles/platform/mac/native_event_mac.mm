/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "platform/mac/native_event_mac.h"

#include <Cocoa/Cocoa.h>

namespace Platform {

bool PossiblyTextTypingEvent(void *event) {
	const auto e = static_cast<NSEvent*>(event);
	if ([e type] != NSEventTypeKeyDown) {
		return false;
	}
	NSEventModifierFlags flags = [e modifierFlags]
		& NSEventModifierFlagDeviceIndependentFlagsMask;
	if ((flags & ~NSEventModifierFlagShift) != 0) {
		return false;
	}
	NSString *text = [e characters];
	const auto length = int([text length]);
	for (auto i = 0; i != length; ++i) {
		const auto utf16 = [text characterAtIndex:i];
		if (utf16 >= 32) {
			return true;
		}
	}
	return false;
}

} // namespace Platform
