/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/tg_ws_proxy/tg_ws_crypto.h"
#include "rabbit/tg_ws_proxy/tg_ws_proxy_constants.h"

#include <QtCore/QtEndian>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace Rabbit::TgWsProxy {
namespace {

bool IsReservedStart(const uchar *bytes) {
	if (bytes[0] == 0xEF) {
		return true;
	}
	const auto first4 = qFromBigEndian<uint32>(bytes);
	if (first4 == 0x48454144U
		|| first4 == 0x504F5354U
		|| first4 == 0x47455420U
		|| first4 == 0xEEEEEEEEU
		|| first4 == 0xDDDDDDDDU
		|| first4 == 0x16030102U
		|| first4 == 0x4F505449U) {
		return true;
	}
	const auto next4 = qFromBigEndian<uint32>(bytes + 4);
	return next4 == 0x00000000U;
}

} // namespace

AesCtrCipher::AesCtrCipher()
: _ctx(EVP_CIPHER_CTX_new()) {
}

AesCtrCipher::~AesCtrCipher() {
	if (_ctx) {
		EVP_CIPHER_CTX_free(_ctx);
		_ctx = nullptr;
	}
}

AesCtrCipher::AesCtrCipher(AesCtrCipher &&other) noexcept
: _ctx(std::exchange(other._ctx, nullptr)) {
}

AesCtrCipher &AesCtrCipher::operator=(AesCtrCipher &&other) noexcept {
	if (this != &other) {
		if (_ctx) {
			EVP_CIPHER_CTX_free(_ctx);
		}
		_ctx = std::exchange(other._ctx, nullptr);
	}
	return *this;
}

bool AesCtrCipher::init(const uchar *key, const uchar *iv) {
	if (!_ctx) {
		return false;
	}
	return EVP_EncryptInit_ex(_ctx, EVP_aes_256_ctr(), nullptr, key, iv) == 1;
}

void AesCtrCipher::update(const char *in, char *out, int len) {
	if (!_ctx || len <= 0) {
		return;
	}
	auto outLen = 0;
	EVP_EncryptUpdate(
		_ctx,
		reinterpret_cast<uchar*>(out),
		&outLen,
		reinterpret_cast<const uchar*>(in),
		len);
}

void AesCtrCipher::update(QByteArray &data) {
	if (data.isEmpty()) {
		return;
	}
	update(data.constData(), data.data(), data.size());
}

QByteArray AesCtrCipher::updated(const QByteArray &data) {
	auto copy = data;
	update(copy);
	return copy;
}

QByteArray Sha256(const QByteArray &data) {
	auto result = QByteArray(32, Qt::Uninitialized);
	SHA256(
		reinterpret_cast<const uchar*>(data.constData()),
		data.size(),
		reinterpret_cast<uchar*>(result.data()));
	return result;
}

std::optional<HandshakeResult> TryHandshake(
		const QByteArray &handshake,
		const QByteArray &secret) {
	if (handshake.size() != kHandshakeLen) {
		return std::nullopt;
	}
	const auto prekeyAndIv = handshake.mid(kSkipLen, kPrekeyLen + kIvLen);
	const auto prekey = prekeyAndIv.left(kPrekeyLen);
	const auto iv = prekeyAndIv.mid(kPrekeyLen, kIvLen);

	const auto key = Sha256(prekey + secret);
	auto cipher = AesCtrCipher();
	if (!cipher.init(
			reinterpret_cast<const uchar*>(key.constData()),
			reinterpret_cast<const uchar*>(iv.constData()))) {
		return std::nullopt;
	}

	const auto decrypted = cipher.updated(handshake);
	const auto protoTag = qFromLittleEndian<uint32>(
		reinterpret_cast<const uchar*>(decrypted.constData() + kProtoTagPos));

	if (protoTag != kProtoTagAbridged
		&& protoTag != kProtoTagIntermediate
		&& protoTag != kProtoTagSecure) {
		return std::nullopt;
	}

	const auto dcIdx = qFromLittleEndian<int16>(
		reinterpret_cast<const uchar*>(decrypted.constData() + kDcIdxPos));

	auto result = HandshakeResult();
	result.dc = std::abs(int(dcIdx));
	result.isMedia = (dcIdx < 0);
	result.protoTag = protoTag;
	result.clientDecPrekeyIv = prekeyAndIv;
	return result;
}

QByteArray GenerateRelayInit(uint32 protoTag, int16 dcIdx) {
	auto rnd = QByteArray(kHandshakeLen, Qt::Uninitialized);
	auto rndBytes = reinterpret_cast<uchar*>(rnd.data());
	while (true) {
		RAND_bytes(rndBytes, kHandshakeLen);
		if (!IsReservedStart(rndBytes)) {
			break;
		}
	}

	const auto encKey = rnd.mid(kSkipLen, kPrekeyLen);
	const auto encIv = rnd.mid(kSkipLen + kPrekeyLen, kIvLen);

	auto encryptor = AesCtrCipher();
	encryptor.init(
		reinterpret_cast<const uchar*>(encKey.constData()),
		reinterpret_cast<const uchar*>(encIv.constData()));

	auto tailPlain = QByteArray(8, Qt::Uninitialized);
	qToLittleEndian(
		protoTag,
		reinterpret_cast<uchar*>(tailPlain.data()));
	qToLittleEndian(
		dcIdx,
		reinterpret_cast<uchar*>(tailPlain.data() + 4));
	RAND_bytes(reinterpret_cast<uchar*>(tailPlain.data() + 6), 2);

	const auto encryptedFull = encryptor.updated(rnd);
	for (auto i = 0; i < 8; ++i) {
		const auto keystreamByte = uchar(encryptedFull[56 + i] ^ rnd[56 + i]);
		rnd[56 + i] = char(uchar(tailPlain[i]) ^ keystreamByte);
	}
	return rnd;
}

std::unique_ptr<CryptoCtx> BuildCryptoCtx(
		const QByteArray &clientDecPrekeyIv,
		const QByteArray &secret,
		const QByteArray &relayInit) {
	auto ctx = std::make_unique<CryptoCtx>();

	const auto cltDecPrekey = clientDecPrekeyIv.left(kPrekeyLen);
	const auto cltDecIv = clientDecPrekeyIv.mid(kPrekeyLen, kIvLen);
	const auto cltDecKey = Sha256(cltDecPrekey + secret);

	auto cltEncPrekeyIv = clientDecPrekeyIv;
	std::reverse(cltEncPrekeyIv.begin(), cltEncPrekeyIv.end());
	const auto cltEncKey = Sha256(cltEncPrekeyIv.left(kPrekeyLen) + secret);
	const auto cltEncIv = cltEncPrekeyIv.mid(kPrekeyLen, kIvLen);

	ctx->cltDec.init(
		reinterpret_cast<const uchar*>(cltDecKey.constData()),
		reinterpret_cast<const uchar*>(cltDecIv.constData()));
	ctx->cltEnc.init(
		reinterpret_cast<const uchar*>(cltEncKey.constData()),
		reinterpret_cast<const uchar*>(cltEncIv.constData()));

	auto zeros64 = QByteArray(kHandshakeLen, 0);
	ctx->cltDec.update(zeros64);

	const auto relayEncKey = relayInit.mid(kSkipLen, kPrekeyLen);
	const auto relayEncIv = relayInit.mid(kSkipLen + kPrekeyLen, kIvLen);

	auto relayDecPrekeyIv = relayInit.mid(kSkipLen, kPrekeyLen + kIvLen);
	std::reverse(relayDecPrekeyIv.begin(), relayDecPrekeyIv.end());
	const auto relayDecKey = relayDecPrekeyIv.left(kKeyLen);
	const auto relayDecIv = relayDecPrekeyIv.mid(kKeyLen, kIvLen);

	ctx->tgEnc.init(
		reinterpret_cast<const uchar*>(relayEncKey.constData()),
		reinterpret_cast<const uchar*>(relayEncIv.constData()));
	ctx->tgDec.init(
		reinterpret_cast<const uchar*>(relayDecKey.constData()),
		reinterpret_cast<const uchar*>(relayDecIv.constData()));

	zeros64.fill(0);
	ctx->tgEnc.update(zeros64);

	return ctx;
}

} // namespace Rabbit::TgWsProxy
