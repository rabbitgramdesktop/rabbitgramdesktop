/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/telemetry/rabbit_telemetry.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QUuid>

#include "core/version.h"
#include "rabbit/settings/rabbit_settings.h"

#if __has_include("../../../../DesktopPrivate/analytics.h")
#include "../../../../DesktopPrivate/analytics.h"
#endif

#ifndef GA4_MEASUREMENT_ID
#define GA4_MEASUREMENT_ID ""
#endif
#ifndef GA4_API_SECRET
#define GA4_API_SECRET ""
#endif

namespace rabbitTelemetry 
{
    void sendTelemetry() {
        if (!RabbitSettings::analytics()) {
            return;
        }
        
        QString measurementId = GA4_MEASUREMENT_ID;
        QString apiSecret = GA4_API_SECRET;
        if (measurementId.isEmpty() || apiSecret.isEmpty()) {
            return;
        }

        QString clientId = RabbitSettings::telemetryClientId();
        if (clientId.isEmpty()) {
            clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            RabbitSettings::setTelemetryClientId(clientId);
        }

        QNetworkAccessManager* manager = new QNetworkAccessManager();
        
        QUrl url(QString("https://www.google-analytics.com/mp/collect?measurement_id=%1&api_secret=%2")
            .arg(measurementId, apiSecret));
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

#if defined Q_OS_WIN
        const auto osName = u"windows"_q;
#elif defined Q_OS_MAC
        const auto osName = u"macos"_q;
#else
        const auto osName = u"linux"_q;
#endif
        
        QJsonObject eventParams;
        eventParams["app_version"] = QString::fromUtf8(AppVersionStr);
        eventParams["os"] = osName;

        QJsonObject eventObj;
        eventObj["name"] = "app_start";
        eventObj["params"] = eventParams;

        QJsonArray eventsArray;
        eventsArray.append(eventObj);

        QJsonObject payload;
        payload["client_id"] = clientId;
        payload["events"] = eventsArray;

        QJsonDocument doc(payload);
        QByteArray data = doc.toJson(QJsonDocument::Compact);

        QNetworkReply* reply = manager->post(request, data);

        QObject::connect(reply, &QNetworkReply::finished, [manager, reply]() {
            reply->deleteLater();
            manager->deleteLater();
        });
    }
}