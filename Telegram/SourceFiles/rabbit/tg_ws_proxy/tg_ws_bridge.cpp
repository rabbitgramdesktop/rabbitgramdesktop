/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/tg_ws_proxy/tg_ws_bridge.h"
#include "rabbit/tg_ws_proxy/tg_ws_proxy_constants.h"

#include <QtCore/QRandomGenerator>
#include <algorithm>

namespace Rabbit::TgWsProxy {
namespace {

constexpr auto kAttemptTimeoutMs = 7000;
constexpr auto kMaxCfAttempts = 4;

} // namespace

TgWsBridge::TgWsBridge(
	std::unique_ptr<QTcpSocket> clientSocket,
	const QByteArray &secret,
	QObject *parent)
: QObject(parent)
, _client(std::move(clientSocket))
, _secret(secret) {
	_attemptTimer.setSingleShot(true);
	connect(&_attemptTimer, &QTimer::timeout, this, [=] {
		handleUpstreamFailed();
	});

	connect(_client.get(), &QTcpSocket::readyRead, this, &TgWsBridge::handleClientReadyRead);
	connect(_client.get(), &QTcpSocket::disconnected, this, &TgWsBridge::handleClientDisconnected);

	connect(&_ws, &TgWsClient::connected, this, &TgWsBridge::handleWsConnected);
	connect(&_ws, &TgWsClient::binaryFrameReceived, this, &TgWsBridge::handleWsBinaryMessage);
	connect(&_ws, &TgWsClient::disconnected, this, &TgWsBridge::handleWsDisconnected);
	connect(&_ws, &TgWsClient::errorOccurred, this, &TgWsBridge::handleWsError);

	connect(&_tcp, &QTcpSocket::connected, this, &TgWsBridge::handleTcpConnected);
	connect(&_tcp, &QTcpSocket::readyRead, this, &TgWsBridge::handleTcpReadyRead);
	connect(&_tcp, &QTcpSocket::disconnected, this, [=] {
		handleUpstreamFailed();
	});
	connect(&_tcp, &QAbstractSocket::errorOccurred, this, [=](QAbstractSocket::SocketError) {
		handleUpstreamFailed();
	});
}

TgWsBridge::~TgWsBridge() {
	closeBoth();
}

void TgWsBridge::closeBoth() {
	if (_closing) {
		return;
	}
	_closing = true;
	_attemptTimer.stop();
	if (_client) {
		_client->disconnect(this);
		_client->disconnectFromHost();
	}
	_ws.disconnect(this);
	_ws.abort();
	_tcp.disconnect(this);
	_tcp.abort();
	// WHY: the owner erases this bridge on finished(); doing it synchronously
	// destroys the sockets while their own signals are still on the stack.
	QMetaObject::invokeMethod(this, [=] {
		Q_EMIT finished();
	}, Qt::QueuedConnection);
}

void TgWsBridge::handleClientDisconnected() {
	closeBoth();
}

void TgWsBridge::handleWsDisconnected() {
	handleUpstreamFailed();
}

void TgWsBridge::handleWsError(const QString &) {
	handleUpstreamFailed();
}

void TgWsBridge::handleUpstreamFailed() {
	if (_closing) {
		return;
	}
	if (_upstreamReady) {
		closeBoth();
		return;
	}
	if (!_attemptActive) {
		return;
	}
	_attemptActive = false;
	_attemptTimer.stop();
	abortUpstream();
	QMetaObject::invokeMethod(this, [=] {
		startNextAttempt();
	}, Qt::QueuedConnection);
}

void TgWsBridge::abortUpstream() {
	_ws.abort();
	auto blocker = QSignalBlocker(&_tcp);
	_tcp.abort();
}

void TgWsBridge::buildAttempts(int dc, bool isMedia, bool isTestDc) {
	_attempts.clear();
	_attemptIndex = 0;

	const auto path = QString::fromLatin1(isTestDc ? kWsPathTest : kWsPath);
	if (dc == 2 || dc == 4) {
		const auto ip = QString::fromLatin1(kDefaultTargetIp);
		for (const auto &domain : WsDomains(dc, isMedia)) {
			_attempts.push_back({ Kind::Ws, ip, domain, path, domain });
			_attempts.push_back({
				Kind::Ws,
				ip,
				domain,
				path,
				QString::fromLatin1(kFrontingSni) });
		}
	}

	if (!isTestDc) {
		auto domains = CfProxyDomains();
		std::shuffle(
			domains.begin(),
			domains.end(),
			*QRandomGenerator::global());
		const auto count = std::min(int(domains.size()), kMaxCfAttempts);
		for (auto i = 0; i != count; ++i) {
			const auto domain = u"kws%1.%2"_q.arg(dc).arg(domains[i]);
			_attempts.push_back({
				Kind::Ws,
				domain,
				domain,
				QString::fromLatin1(kWsPath),
				domain });
		}
	}

	const auto knownDc = (dc >= 1 && dc <= 5) || (dc == 203);
	if (knownDc) {
		_attempts.push_back({
			Kind::Tcp,
			DcDefaultIp(dc, isTestDc),
			QString(),
			QString(),
			QString() });
	}
}

void TgWsBridge::startNextAttempt() {
	if (_closing || _upstreamReady) {
		return;
	}
	if (_attemptIndex >= int(_attempts.size())) {
		closeBoth();
		return;
	}
	const auto &attempt = _attempts[_attemptIndex++];
	_attemptActive = true;
	_activeKind = attempt.kind;
	_attemptTimer.start(kAttemptTimeoutMs);
	if (attempt.kind == Kind::Ws) {
		_ws.connectToHost(
			attempt.host,
			attempt.domain,
			attempt.path,
			attempt.sni);
	} else {
		_tcp.connectToHost(attempt.host, 443);
	}
}

void TgWsBridge::handleClientReadyRead() {
	if (!_client) {
		return;
	}
	const auto available = _client->readAll();
	if (available.isEmpty()) {
		return;
	}

	if (!_handshakeDone) {
		_clientInitBuffer.append(available);
		if (_clientInitBuffer.size() < kHandshakeLen) {
			return;
		}

		const auto handshake = _clientInitBuffer.left(kHandshakeLen);
		const auto remaining = _clientInitBuffer.mid(kHandshakeLen);
		_clientInitBuffer.clear();

		const auto result = TryHandshake(handshake, _secret);
		if (!result) {
			closeBoth();
			return;
		}

		auto dc = result->dc;
		const auto isTestDc = (dc >= 10000);
		if (isTestDc) {
			dc -= 10000;
		}
		const auto isMedia = result->isMedia;
		const auto dcIdx = int16(isMedia ? -dc : dc);

		_relayInit = GenerateRelayInit(result->protoTag, dcIdx);
		_crypto = BuildCryptoCtx(result->clientDecPrekeyIv, _secret, _relayInit);
		_splitter = std::make_unique<MsgSplitter>(result->protoTag);
		_handshakeDone = true;

		buildAttempts(dc, isMedia, isTestDc);
		startNextAttempt();

		if (!remaining.isEmpty()) {
			processClientData(remaining);
		}
		return;
	}

	processClientData(available);
}

void TgWsBridge::processClientData(const QByteArray &data) {
	if (!_upstreamReady) {
		_pendingClientBuffer.append(data);
		return;
	}

	auto plain = data;
	_crypto->cltDec.update(plain);
	auto cipher = plain;
	_crypto->tgEnc.update(cipher);

	if (_activeKind == Kind::Tcp) {
		_tcp.write(cipher);
		return;
	}
	const auto packets = _splitter->split(cipher, plain);
	for (const auto &packet : packets) {
		_ws.sendBinary(packet);
	}
}

void TgWsBridge::deliverToClient(QByteArray data) {
	if (!_client || data.isEmpty()) {
		return;
	}
	_crypto->tgDec.update(data);
	_crypto->cltEnc.update(data);
	_client->write(data);
}

void TgWsBridge::handleWsConnected() {
	if (_closing || !_attemptActive) {
		return;
	}
	_attemptActive = false;
	_attemptTimer.stop();
	_upstreamReady = true;
	_ws.sendBinary(_relayInit);

	if (!_pendingClientBuffer.isEmpty()) {
		const auto pending = std::exchange(_pendingClientBuffer, QByteArray());
		processClientData(pending);
	}
}

void TgWsBridge::handleTcpConnected() {
	if (_closing || !_attemptActive) {
		return;
	}
	_attemptActive = false;
	_attemptTimer.stop();
	_upstreamReady = true;
	_tcp.write(_relayInit);

	if (!_pendingClientBuffer.isEmpty()) {
		const auto pending = std::exchange(_pendingClientBuffer, QByteArray());
		processClientData(pending);
	}
}

void TgWsBridge::handleTcpReadyRead() {
	if (!_upstreamReady) {
		return;
	}
	deliverToClient(_tcp.readAll());
}

void TgWsBridge::handleWsBinaryMessage(const QByteArray &data) {
	if (!_upstreamReady) {
		return;
	}
	deliverToClient(data);
}

} // namespace Rabbit::TgWsProxy
