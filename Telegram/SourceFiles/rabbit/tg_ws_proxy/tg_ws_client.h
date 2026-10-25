/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtNetwork/QSslSocket>

namespace Rabbit::TgWsProxy {

class TgWsClient final : public QObject {
	Q_OBJECT

public:
	enum class State {
		Connecting,
		Handshaking,
		Connected,
		Closing,
		Closed,
	};

	explicit TgWsClient(QObject *parent = nullptr);
	~TgWsClient();

	void connectToHost(
		const QString &targetIp,
		const QString &domain,
		const QString &path = u"/apiws"_q,
		const QString &sni = QString());

	void sendBinary(const QByteArray &payload);
	void close();
	void abort();

	[[nodiscard]] State state() const {
		return _state;
	}

Q_SIGNALS:
	void connected();
	void binaryFrameReceived(const QByteArray &data);
	void disconnected();
	void errorOccurred(const QString &error);

private:
	void handleEncrypted();
	void handleReadyRead();
	void processHttpHandshake();
	void processFrames();
	void sendPong(const QByteArray &data);

	QSslSocket _socket;
	State _state = State::Closed;
	QString _domain;
	QString _path;
	QByteArray _expectedAcceptKey;
	QByteArray _readBuffer;
	QByteArray _fragmentBuffer;
};

} // namespace Rabbit::TgWsProxy
