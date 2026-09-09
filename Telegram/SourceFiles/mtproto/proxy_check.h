/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "mtproto/connection_abstract.h"

namespace MTP {

using ProxyCheckConnection = details::ConnectionPointer;

void ResetProxyCheckers(
	ProxyCheckConnection &v4,
	ProxyCheckConnection &v6);
void DropProxyChecker(
	ProxyCheckConnection &v4,
	ProxyCheckConnection &v6,
	not_null<details::AbstractConnection*> raw);
[[nodiscard]] bool HasProxyCheckers(
	const ProxyCheckConnection &v4,
	const ProxyCheckConnection &v6);
void StartProxyCheck(
	not_null<Instance*> mtproto,
	const ProxyData &proxy,
	bool tryIPv6,
	ProxyCheckConnection &v4,
	ProxyCheckConnection &v6,
	Fn<void(details::AbstractConnection *raw, int ping)> done,
	Fn<void(details::AbstractConnection *raw)> fail);

} // namespace MTP
