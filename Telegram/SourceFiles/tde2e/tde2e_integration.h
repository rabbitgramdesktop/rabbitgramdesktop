/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace TdE2E {

struct UserId;
struct PublicKey;

[[nodiscard]] UserId MakeUserId(not_null<UserData*> user);
[[nodiscard]] UserId MakeUserId(::UserId id);

[[nodiscard]] MTPint256 PublicKeyToMTP(const PublicKey &key);

} // namespace TdE2E
