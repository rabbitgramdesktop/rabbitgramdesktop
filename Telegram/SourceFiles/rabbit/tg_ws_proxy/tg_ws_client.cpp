/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/tg_ws_proxy/tg_ws_client.h"

#include <QtCore/QtEndian>
#include <QtNetwork/QSslError>
#include <openssl/rand.h>

namespace Rabbit::TgWsProxy {
namespace {

constexpr auto kOpCont = 0x0;
constexpr auto kOpText = 0x1;
constexpr auto kOpBinary = 0x2;
constexpr auto kOpClose = 0x8;
constexpr auto kOpPing = 0x9;
constexpr auto kOpPong = 0xA;

} // namespace

TgWsClient::TgWsClient(QObject *parent)
: QObject(parent) {
	connect(&_socket, &QSslSocket::encrypted, this, &TgWsClient::handleEncrypted);
	connect(&_socket, &QSslSocket::readyRead, this, &TgWsClient::handleReadyRead);
	connect(&_socket, &QSslSocket::disconnected, this, [=] {
		_state = State::Closed;
		_readBuffer.clear();
		_fragmentBuffer.clear();
		Q_EMIT disconnected();
	});
	connect(
		&_socket,
		QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
		this,
		[=](const QList<QSslError> &) {
			_socket.ignoreSslErrors();
		});
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(&_socket, &QSslSocket::errorOccurred, this, [=](QAbstractSocket::SocketError) {
		if (_state != State::Closed) {
			Q_EMIT errorOccurred(_socket.errorString());
		}
	});
#endif
}

TgWsClient::~TgWsClient() {
	close();
}

void TgWsClient::connectToHost(
		const QString &targetIp,
		const QString &domain,
		const QString &path,
		const QString &sni) {
	abort();
	_domain = domain;
	_path = path.isEmpty() ? u"/apiws"_q : path;
	_state = State::Connecting;
	_readBuffer.clear();
	_fragmentBuffer.clear();

	const auto peerName = sni.isEmpty() ? domain : sni;
	_socket.setPeerVerifyName(peerName);
	_socket.setPeerVerifyMode(QSslSocket::VerifyNone);
	_socket.ignoreSslErrors();
	_socket.connectToHostEncrypted(targetIp, 443, peerName);
}

void TgWsClient::abort() {
	_state = State::Closed;
	_readBuffer.clear();
	_fragmentBuffer.clear();
	const auto blocker = QSignalBlocker(&_socket);
	_socket.abort();
}

void TgWsClient::close() {
	if (_state == State::Closed) {
		return;
	}
	_state = State::Closing;
	_socket.disconnectFromHost();
}

void TgWsClient::handleEncrypted() {
	if (_state != State::Connecting) {
		return;
	}
	_state = State::Handshaking;

	auto nonce = QByteArray(16, Qt::Uninitialized);
	RAND_bytes(reinterpret_cast<uchar*>(nonce.data()), 16);
	const auto wsKey = nonce.toBase64();

	const auto request = u"GET %1 HTTP/1.1\r\n"_q.arg(_path)
		+ u"Host: %1\r\n"_q.arg(_domain)
		+ u"User-Agent: Mozilla/5.0\r\n"_q
		+ u"Origin: https://web.telegram.org\r\n"_q
		+ u"Upgrade: websocket\r\n"_q
		+ u"Connection: Upgrade\r\n"_q
		+ u"Sec-WebSocket-Key: %1\r\n"_q.arg(QString::fromLatin1(wsKey))
		+ u"Sec-WebSocket-Version: 13\r\n"_q
		+ u"Sec-WebSocket-Protocol: binary\r\n\r\n"_q;

	_socket.write(request.toLatin1());
}

void TgWsClient::handleReadyRead() {
	_readBuffer.append(_socket.readAll());
	if (_state == State::Handshaking) {
		processHttpHandshake();
	}
	if (_state == State::Connected) {
		processFrames();
	}
}

void TgWsClient::processHttpHandshake() {
	const auto headerEnd = _readBuffer.indexOf("\r\n\r\n");
	if (headerEnd == -1) {
		return;
	}
	const auto headerBytes = _readBuffer.left(headerEnd);
	_readBuffer.remove(0, headerEnd + 4);

	const auto firstLineEnd = headerBytes.indexOf("\r\n");
	const auto statusLine = (firstLineEnd != -1)
		? headerBytes.left(firstLineEnd)
		: headerBytes;

	if (!statusLine.contains("101")) {
		_state = State::Closed;
		_socket.disconnectFromHost();
		Q_EMIT errorOccurred(u"WebSocket upgrade rejected: "_q + QString::fromLatin1(statusLine));
		return;
	}

	_state = State::Connected;
	Q_EMIT connected();

	if (!_readBuffer.isEmpty()) {
		processFrames();
	}
}

void TgWsClient::sendBinary(const QByteArray &payload) {
	if (_state != State::Connected) {
		return;
	}
	auto frame = QByteArray();
	frame.reserve(14 + payload.size());
	frame.append(char(0x80 | kOpBinary));

	const auto size = payload.size();
	if (size < 126) {
		frame.append(char(0x80 | size));
	} else if (size <= 0xFFFF) {
		frame.append(char(0x80 | 126));
		auto len16 = qToBigEndian<uint16>(size);
		frame.append(reinterpret_cast<const char*>(&len16), 2);
	} else {
		frame.append(char(0x80 | 127));
		auto len64 = qToBigEndian<uint64>(size);
		frame.append(reinterpret_cast<const char*>(&len64), 8);
	}

	auto mask = QByteArray(4, Qt::Uninitialized);
	RAND_bytes(reinterpret_cast<uchar*>(mask.data()), 4);
	frame.append(mask);

	auto maskedPayload = payload;
	const auto maskData = reinterpret_cast<const uchar*>(mask.constData());
	auto payloadData = reinterpret_cast<uchar*>(maskedPayload.data());
	for (auto i = 0; i < size; ++i) {
		payloadData[i] ^= maskData[i % 4];
	}
	frame.append(maskedPayload);

	_socket.write(frame);
}

void TgWsClient::sendPong(const QByteArray &data) {
	auto frame = QByteArray();
	frame.append(char(0x80 | kOpPong));
	frame.append(char(0x80 | (data.size() & 0x7F)));

	auto mask = QByteArray(4, Qt::Uninitialized);
	RAND_bytes(reinterpret_cast<uchar*>(mask.data()), 4);
	frame.append(mask);

	auto masked = data;
	const auto maskData = reinterpret_cast<const uchar*>(mask.constData());
	auto payloadData = reinterpret_cast<uchar*>(masked.data());
	for (auto i = 0; i < data.size(); ++i) {
		payloadData[i] ^= maskData[i % 4];
	}
	frame.append(masked);
	_socket.write(frame);
}

void TgWsClient::processFrames() {
	while (_readBuffer.size() >= 2) {
		const auto data = reinterpret_cast<const uchar*>(_readBuffer.constData());
		const auto fin = (data[0] & 0x80) != 0;
		const auto opcode = data[0] & 0x0F;
		const auto isMasked = (data[1] & 0x80) != 0;
		auto payloadLen = uint64(data[1] & 0x7F);
		auto headerLen = 2;

		if (payloadLen == 126) {
			if (_readBuffer.size() < 4) {
				return;
			}
			payloadLen = qFromBigEndian<uint16>(data + 2);
			headerLen += 2;
		} else if (payloadLen == 127) {
			if (_readBuffer.size() < 10) {
				return;
			}
			payloadLen = qFromBigEndian<uint64>(data + 2);
			headerLen += 8;
		}

		if (isMasked) {
			headerLen += 4;
		}

		if (uint64(_readBuffer.size()) < uint64(headerLen) + payloadLen) {
			return;
		}

		auto payload = _readBuffer.mid(headerLen, int(payloadLen));
		if (isMasked) {
			const auto maskOffset = headerLen - 4;
			const auto maskData = data + maskOffset;
			auto payloadData = reinterpret_cast<uchar*>(payload.data());
			for (auto i = 0; i < payload.size(); ++i) {
				payloadData[i] ^= maskData[i % 4];
			}
		}

		_readBuffer.remove(0, headerLen + int(payloadLen));

		if (opcode == kOpPing) {
			sendPong(payload);
		} else if (opcode == kOpClose) {
			close();
			return;
		} else if (opcode == kOpBinary || opcode == kOpCont) {
			_fragmentBuffer.append(payload);
			if (fin) {
				auto message = std::exchange(_fragmentBuffer, QByteArray());
				Q_EMIT binaryFrameReceived(message);
			}
		}
	}
}

} // namespace Rabbit::TgWsProxy
