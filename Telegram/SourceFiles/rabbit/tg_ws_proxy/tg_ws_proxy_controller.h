/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "rabbit/tg_ws_proxy/tg_ws_proxy_server.h"
#include "mtproto/mtproto_proxy_data.h"

#include <QtCore/QObject>
#include <rpl/producer.h>

namespace Rabbit::TgWsProxy {

class Controller final : public QObject {
	Q_OBJECT

public:
	[[nodiscard]] static Controller &Instance();

	bool start();
	void stop();
	void applyIfEnabled();

	[[nodiscard]] bool isRunning() const;
	[[nodiscard]] MTP::ProxyData proxyData() const;
	[[nodiscard]] rpl::producer<bool> runningValue() const;

private:
	Controller();

	TgWsProxyServer _server;
	MTP::ProxyData _currentProxy;
	MTP::ProxyData _savedProxy;
	MTP::ProxyData::Settings _savedSettings = MTP::ProxyData::Settings::System;
	bool _hasSavedState = false;
	rpl::event_stream<bool> _runningChanges;
};

} // namespace Rabbit::TgWsProxy
