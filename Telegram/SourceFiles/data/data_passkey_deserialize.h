/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/

#pragma once

namespace Data::Passkey {

struct RelyingParty {
	QString id;
	QString name;
};

struct User {
	QByteArray id;
	QString name;
	QString displayName;
};

struct CredentialParameter {
	QString type;
	int alg = 0;
};

struct RegisterData {
	RelyingParty rp;
	User user;
	QByteArray challenge;
	std::vector<CredentialParameter> pubKeyCredParams;
	int timeout = 60000;
};

struct Credential {
	QByteArray id;
	QString type;
};

struct LoginData {
	QByteArray challenge;
	QString rpId;
	std::vector<Credential> allowCredentials;
	QString userVerification;
	int timeout = 60000;
};

[[nodiscard]] std::optional<RegisterData> DeserializeRegisterData(
	const QByteArray &jsonData);

[[nodiscard]] std::optional<LoginData> DeserializeLoginData(
	const QByteArray &jsonData);

[[nodiscard]] std::string SerializeClientDataCreate(
	const QByteArray &challenge);

[[nodiscard]] std::string SerializeClientDataGet(
	const QByteArray &challenge);

} // namespace Data::Passkey
