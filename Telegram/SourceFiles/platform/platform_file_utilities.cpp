/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "platform/platform_file_utilities.h"

#include "test/test_launch_fuse.h"

namespace Platform::File {

void UnsafeOpenUrl(const QString &url) {
	if (Test::BlockLaunch(u"UnsafeOpenUrl"_q, url)) {
		return;
	}
	Unfused::UnsafeOpenUrl(url);
}

void UnsafeOpenEmailLink(const QString &email) {
	if (Test::BlockLaunch(u"UnsafeOpenEmailLink"_q, email)) {
		return;
	}
	Unfused::UnsafeOpenEmailLink(email);
}

bool UnsafeShowOpenWithDropdown(const QString &filepath) {
	if (Test::BlockLaunch(u"UnsafeShowOpenWithDropdown"_q, filepath)) {
		return true;
	}
	return Unfused::UnsafeShowOpenWithDropdown(filepath);
}

bool UnsafeShowOpenWith(const QString &filepath) {
	if (Test::BlockLaunch(u"UnsafeShowOpenWith"_q, filepath)) {
		return true;
	}
	return Unfused::UnsafeShowOpenWith(filepath);
}

void UnsafeLaunch(const QString &filepath) {
	if (Test::BlockLaunch(u"UnsafeLaunch"_q, filepath)) {
		return;
	}
	Unfused::UnsafeLaunch(filepath);
}

} // namespace Platform::File
