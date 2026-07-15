/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/telemetry/rabbit_telemetry.h"

#include "core/version.h"
#include "rabbit/settings/rabbit_settings.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUuid>
#include <QtCore/QSysInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#if __has_include("../../../../DesktopPrivate/analytics.h")
#include "../../../../DesktopPrivate/analytics.h"
#endif

#ifndef GA4_MEASUREMENT_ID
#define GA4_MEASUREMENT_ID ""
#endif
#ifndef GA4_API_SECRET
#define GA4_API_SECRET ""
#endif

namespace RabbitTelemetry {

void sendTelemetry() {
	if (!RabbitSettings::analytics()) {
		return;
	}

	const auto measurementId = QString(GA4_MEASUREMENT_ID);
	const auto apiSecret = QString(GA4_API_SECRET);
	if (measurementId.isEmpty() || apiSecret.isEmpty()) {
		return;
	}

	auto clientId = RabbitSettings::telemetryClientId();
	if (clientId.isEmpty()) {
		clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
		RabbitSettings::setTelemetryClientId(clientId);
	}

	const auto manager = new QNetworkAccessManager();

	const auto url = QUrl(u"https://www.google-analytics.com/mp/collect?measurement_id=%1&api_secret=%2"_q
		.arg(measurementId, apiSecret));

	auto request = QNetworkRequest(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_q);

#ifdef Q_OS_WIN
	const auto osName = u"windows"_q;
#elif defined Q_OS_MAC
	const auto osName = u"macos"_q;
#else
	const auto osName = u"linux"_q;
#endif

	auto eventParams = QJsonObject();
	if (RabbitSettings::telemetryAppVersion()) {
		eventParams[u"app_version"_q] = QString::fromUtf8(AppVersionStr);
	}
	if (RabbitSettings::telemetryOs()) {
		eventParams[u"os"_q] = osName;
	}
	if (RabbitSettings::telemetryCpuArch()) {
		eventParams[u"cpu_arch"_q] = QSysInfo::currentCpuArchitecture();
	}

	if (RabbitSettings::telemetryFeatures()) {
		const auto features = RabbitSettings::JsonSettings::GetFeaturesTelemetryData();
		for (auto it = features.constBegin(); it != features.constEnd(); ++it) {
			eventParams.insert(it.key(), it.value());
		}
	}

	auto eventObj = QJsonObject();
	eventObj[u"name"_q] = u"app_start"_q;
	eventObj[u"params"_q] = eventParams;

	auto eventsArray = QJsonArray();
	eventsArray.append(eventObj);

	auto payload = QJsonObject();
	payload[u"client_id"_q] = clientId;
	payload[u"events"_q] = eventsArray;

	const auto doc = QJsonDocument(payload);
	const auto data = doc.toJson(QJsonDocument::Compact);

	const auto reply = manager->post(request, data);

	QObject::connect(reply, &QNetworkReply::finished, [=] {
		reply->deleteLater();
		manager->deleteLater();
	});
}

} // namespace RabbitTelemetry