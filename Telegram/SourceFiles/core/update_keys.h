/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QByteArray>

namespace Core::Updates {

// The pinned root Ed25519 public key and the built-in key manifest with
// its detached root signature, embedded at build time from the files in
// Telegram/Resources/update/ through a generated header.

[[nodiscard]] QByteArray RootPublicKeyPem();
[[nodiscard]] QByteArray EmbeddedManifest();
[[nodiscard]] QByteArray EmbeddedManifestSignature();

} // namespace Core::Updates
