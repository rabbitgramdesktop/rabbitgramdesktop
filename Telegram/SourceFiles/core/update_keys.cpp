/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "core/update_keys.h"

#include "update_keys_data.h"

namespace Core::Updates {

QByteArray RootPublicKeyPem() {
	return QByteArray::fromRawData(
		reinterpret_cast<const char*>(details::kRootPublicKeyPem),
		sizeof(details::kRootPublicKeyPem));
}

QByteArray EmbeddedManifest() {
	return QByteArray::fromRawData(
		reinterpret_cast<const char*>(details::kEmbeddedManifest),
		sizeof(details::kEmbeddedManifest));
}

QByteArray EmbeddedManifestSignature() {
	return QByteArray::fromRawData(
		reinterpret_cast<const char*>(details::kEmbeddedManifestSig),
		sizeof(details::kEmbeddedManifestSig));
}

} // namespace Core::Updates
