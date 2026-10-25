/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "rabbit/tg_ws_proxy/tg_ws_bridge.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtNetwork/QTcpServer>
#include <vector>

namespace Rabbit::TgWsProxy {

class TgWsProxyServer final : public QObject {
	Q_OBJECT

public:
	explicit TgWsProxyServer(QObject *parent = nullptr);
	~TgWsProxyServer();

	bool start(uint16 preferredPort = 0);
	void stop();

	[[nodiscard]] bool isListening() const;
	[[nodiscard]] uint16 port() const;
	[[nodiscard]] QString secretHex() const;
	[[nodiscard]] QByteArray secretBytes() const;

private:
	void handleNewConnection();

	QTcpServer _server;
	QByteArray _secret;
	std::vector<std::unique_ptr<TgWsBridge>> _bridges;
};

} // namespace Rabbit::TgWsProxy
