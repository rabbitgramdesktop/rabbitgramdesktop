/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "rabbit/tg_ws_proxy/tg_ws_client.h"
#include "rabbit/tg_ws_proxy/tg_ws_crypto.h"
#include "rabbit/tg_ws_proxy/tg_ws_msg_splitter.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>
#include <memory>
#include <vector>

namespace Rabbit::TgWsProxy {

class TgWsBridge final : public QObject {
	Q_OBJECT

public:
	TgWsBridge(
		std::unique_ptr<QTcpSocket> clientSocket,
		const QByteArray &secret,
		QObject *parent = nullptr);
	~TgWsBridge();

Q_SIGNALS:
	void finished();

private:
	enum class Kind {
		Ws,
		Tcp,
	};
	struct Attempt {
		Kind kind = Kind::Ws;
		QString host;
		QString domain;
		QString path;
		QString sni;
	};

	void handleClientReadyRead();
	void handleClientDisconnected();
	void handleWsConnected();
	void handleWsBinaryMessage(const QByteArray &data);
	void handleWsDisconnected();
	void handleWsError(const QString &error);
	void handleTcpConnected();
	void handleTcpReadyRead();
	void handleUpstreamFailed();
	void buildAttempts(int dc, bool isMedia, bool isTestDc);
	void startNextAttempt();
	void abortUpstream();
	void deliverToClient(QByteArray data);
	void processClientData(const QByteArray &data);
	void closeBoth();

	std::unique_ptr<QTcpSocket> _client;
	TgWsClient _ws;
	QTcpSocket _tcp;
	QTimer _attemptTimer;
	QByteArray _secret;
	QByteArray _clientInitBuffer;
	QByteArray _pendingClientBuffer;
	QByteArray _relayInit;

	std::vector<Attempt> _attempts;
	int _attemptIndex = 0;
	Kind _activeKind = Kind::Ws;
	std::unique_ptr<CryptoCtx> _crypto;
	std::unique_ptr<MsgSplitter> _splitter;
	bool _handshakeDone = false;
	bool _attemptActive = false;
	bool _upstreamReady = false;
	bool _closing = false;
};

} // namespace Rabbit::TgWsProxy
