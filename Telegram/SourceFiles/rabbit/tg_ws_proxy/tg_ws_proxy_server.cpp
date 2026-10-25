/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/tg_ws_proxy/tg_ws_proxy_server.h"
#include "rabbit/tg_ws_proxy/tg_ws_proxy_constants.h"

#include <openssl/rand.h>

namespace Rabbit::TgWsProxy {

TgWsProxyServer::TgWsProxyServer(QObject *parent)
: QObject(parent) {
	connect(&_server, &QTcpServer::newConnection, this, &TgWsProxyServer::handleNewConnection);
}

TgWsProxyServer::~TgWsProxyServer() {
	stop();
}

bool TgWsProxyServer::start(uint16 preferredPort) {
	if (isListening()) {
		return true;
	}

	_secret = QByteArray(16, Qt::Uninitialized);
	RAND_bytes(reinterpret_cast<uchar*>(_secret.data()), 16);

	const auto portToTry = preferredPort ? preferredPort : kDefaultPort;
	if (_server.listen(QHostAddress::LocalHost, portToTry)) {
		return true;
	}

	return _server.listen(QHostAddress::LocalHost, 0);
}

void TgWsProxyServer::stop() {
	_server.close();
	_bridges.clear();
}

bool TgWsProxyServer::isListening() const {
	return _server.isListening();
}

uint16 TgWsProxyServer::port() const {
	return _server.serverPort();
}

QString TgWsProxyServer::secretHex() const {
	return QString::fromLatin1(_secret.toHex());
}

QByteArray TgWsProxyServer::secretBytes() const {
	return _secret;
}

void TgWsProxyServer::handleNewConnection() {
	while (_server.hasPendingConnections()) {
		auto client = _server.nextPendingConnection();
		if (!client) {
			continue;
		}

		auto bridge = std::make_unique<TgWsBridge>(
			std::unique_ptr<QTcpSocket>(client),
			_secret,
			this);

		const auto raw = bridge.get();
		connect(raw, &TgWsBridge::finished, this, [=] {
			const auto it = std::find_if(
				_bridges.begin(),
				_bridges.end(),
				[&](const std::unique_ptr<TgWsBridge> &b) {
					return b.get() == raw;
				});
			if (it != _bridges.end()) {
				_bridges.erase(it);
			}
		});

		_bridges.push_back(std::move(bridge));
	}
}

} // namespace Rabbit::TgWsProxy
