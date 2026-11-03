/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QMutex>
#include <QtCore/QRandomGenerator>
#include <algorithm>

namespace Rabbit::TgWsProxy {

inline constexpr auto kHandshakeLen = 64;
inline constexpr auto kSkipLen = 8;
inline constexpr auto kPrekeyLen = 32;
inline constexpr auto kKeyLen = 32;
inline constexpr auto kIvLen = 16;
inline constexpr auto kProtoTagPos = 56;
inline constexpr auto kDcIdxPos = 60;

inline constexpr auto kProtoTagAbridged = 0xEFEFEFEFU;
inline constexpr auto kProtoTagIntermediate = 0xEEEEEEEEU;
inline constexpr auto kProtoTagSecure = 0xDDDDDDDDU;

inline constexpr auto kDefaultPort = 1443;
inline constexpr auto kWsPath = "/apiws";
inline constexpr auto kWsPathTest = "/apiws_test";
inline constexpr auto kFrontingSni = "sprinthost.ru";
inline constexpr auto kDefaultTargetIp = "149.154.167.220";

[[nodiscard]] inline QStringList WsDomains(int dc, bool isMedia) {
	if (dc == 203) {
		dc = 2;
	}
	if (!isMedia) {
		return { u"kws%1.web.telegram.org"_q.arg(dc) };
	}
	return {
		u"kws%1-1.web.telegram.org"_q.arg(dc),
		u"kws%1.web.telegram.org"_q.arg(dc)
	};
}

[[nodiscard]] inline QString DcDefaultIp(int dc, bool isTestDc) {
	if (isTestDc) {
		switch (dc) {
		case 1: return u"149.154.175.10"_q;
		case 2: return u"149.154.167.40"_q;
		case 3: return u"149.154.175.117"_q;
		default: return u"149.154.175.10"_q;
		}
	}
	switch (dc) {
	case 1: return u"149.154.175.50"_q;
	case 2: return u"149.154.167.51"_q;
	case 3: return u"149.154.175.100"_q;
	case 4: return u"149.154.167.91"_q;
	case 5: return u"149.154.171.5"_q;
	case 203: return u"91.105.192.100"_q;
	default: return u"149.154.167.220"_q;
	}
}

[[nodiscard]] inline QString DecodeCfDomain(const QString &encoded) {
	if (!encoded.endsWith(u".com"_q)) {
		return encoded;
	}
	auto body = encoded.chopped(4);
	auto letters = 0;
	for (const auto ch : body) {
		if (ch.isLetter()) {
			++letters;
		}
	}
	for (auto &ch : body) {
		if (!ch.isLetter()) {
			continue;
		}
		const auto base = (ch.unicode() > '`') ? 'a' : 'A';
		ch = QChar(base + ((ch.unicode() - base - letters) % 26 + 26) % 26);
	}
	return body + u".co.uk"_q;
}

[[nodiscard]] inline QStringList CfProxyDomains() {
	static const auto result = [] {
		const auto encoded = QStringList{
			u"virkgj.com"_q, u"vmmzovy.com"_q, u"mkuosckvso.com"_q,
			u"zaewayzmplad.com"_q, u"twdmbzcm.com"_q, u"awzwsldi.com"_q,
			u"clngqrflngqin.com"_q, u"tjacxbqtj.com"_q, u"bxaxtxmrw.com"_q,
			u"dmohrsgmohcrwb.com"_q, u"vwbmtmoi.com"_q, u"khgrre.com"_q,
			u"ulihssf.com"_q, u"tmhqsdqmfpmk.com"_q, u"xwuwoqbm.com"_q,
			u"orgcnunpj.com"_q, u"zhkuldz.com"_q, u"zypoljnslxa.com"_q,
			u"efabnxaowuzs.com"_q, u"zaftuzsftqdq.com"_q,
		};
		auto list = QStringList();
		for (const auto &item : encoded) {
			list.push_back(DecodeCfDomain(item));
		}
		return list;
	}();
	return result;
}

class Balancer final {
public:
	static Balancer &Instance() {
		static Balancer instance;
		return instance;
	}

	QStringList getDomainsForDc(int dc) {
		const auto all = CfProxyDomains();
		if (all.isEmpty()) {
			return {};
		}
		auto locker = QMutexLocker(&_mutex);
		auto result = QStringList();
		const auto it = _dcToDomain.find(dc);
		const auto current = (it != _dcToDomain.end()) ? it.value() : QString();
		if (!current.isEmpty() && all.contains(current)) {
			result.push_back(current);
		}
		auto shuffled = all;
		std::shuffle(
			shuffled.begin(),
			shuffled.end(),
			*QRandomGenerator::global());
		for (const auto &domain : shuffled) {
			if (domain != current) {
				result.push_back(domain);
			}
		}
		return result;
	}

	void updateDomainForDc(int dc, const QString &domain) {
		if (domain.isEmpty()) {
			return;
		}
		auto locker = QMutexLocker(&_mutex);
		_dcToDomain[dc] = domain;
	}

private:
	Balancer() {
		const auto all = CfProxyDomains();
		if (!all.isEmpty()) {
			const auto dcs = { 1, 2, 3, 4, 5, 203 };
			for (const auto dc : dcs) {
				const auto idx = QRandomGenerator::global()->bounded(all.size());
				_dcToDomain[dc] = all[idx];
			}
		}
	}

	QMutex _mutex;
	QMap<int, QString> _dcToDomain;
};

} // namespace Rabbit::TgWsProxy

