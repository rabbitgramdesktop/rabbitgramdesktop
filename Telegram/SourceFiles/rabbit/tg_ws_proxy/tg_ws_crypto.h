/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QByteArray>
#include <memory>
#include <optional>

struct evp_cipher_ctx_st;

namespace Rabbit::TgWsProxy {

class AesCtrCipher final {
public:
	AesCtrCipher();
	~AesCtrCipher();

	AesCtrCipher(AesCtrCipher &&other) noexcept;
	AesCtrCipher &operator=(AesCtrCipher &&other) noexcept;

	AesCtrCipher(const AesCtrCipher &) = delete;
	AesCtrCipher &operator=(const AesCtrCipher &) = delete;

	bool init(const uchar *key, const uchar *iv);
	void update(const char *in, char *out, int len);
	void update(QByteArray &data);
	[[nodiscard]] QByteArray updated(const QByteArray &data);

private:
	evp_cipher_ctx_st *_ctx = nullptr;
};

struct CryptoCtx {
	AesCtrCipher cltDec;
	AesCtrCipher cltEnc;
	AesCtrCipher tgEnc;
	AesCtrCipher tgDec;
};

struct HandshakeResult {
	int dc = 0;
	bool isMedia = false;
	uint32 protoTag = 0;
	QByteArray clientDecPrekeyIv;
};

[[nodiscard]] QByteArray Sha256(const QByteArray &data);

[[nodiscard]] std::optional<HandshakeResult> TryHandshake(
	const QByteArray &handshake,
	const QByteArray &secret);

[[nodiscard]] QByteArray GenerateRelayInit(uint32 protoTag, int16 dcIdx);

[[nodiscard]] std::unique_ptr<CryptoCtx> BuildCryptoCtx(
	const QByteArray &clientDecPrekeyIv,
	const QByteArray &secret,
	const QByteArray &relayInit);

} // namespace Rabbit::TgWsProxy
