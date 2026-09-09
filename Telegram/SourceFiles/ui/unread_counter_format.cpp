/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/unread_counter_format.h"

QString FormatUnreadCounter(
		int unreadCounter,
		bool hasMentionOrReaction,
		bool narrow) {
	if (unreadCounter <= 0) {
		return QString();
	}
	if (!narrow) {
		return QString::number(unreadCounter);
	}
	if (hasMentionOrReaction && (unreadCounter > 999)) {
		return u"99+"_q;
	}
	if (unreadCounter > 999999) {
		return u"99999+"_q;
	}
	return QString::number(unreadCounter);
}