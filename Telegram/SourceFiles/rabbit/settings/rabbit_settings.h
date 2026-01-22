/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <rpl/producer.h>

#include <QtCore/QVariant>
#include <QtCore/QJsonArray>

namespace RabbitSettings {

const auto DEFAULT_ICON = QString("default");
const auto ANGEL_ICON = QString("angel");
const auto BLACKOUT_ICON = QString("blackout");
const auto COLOR_ICON = QString("color");
const auto IMPOSTOR_ICON = QString("impostor");
const auto MATRIX_ICON = QString("matrix");
const auto MOON_ICON = QString("moon");
const auto OLD_ICON = QString("old");
const auto SEASONAL_ICON = QString("seasonal");
const auto TWITCH_ICON = QString("twitch");

namespace JsonSettings {

void Start();
void Load();
void Write();
void Finish();

[[nodiscard]] QVariant Get(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);
[[nodiscard]] QVariant GetWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);
[[nodiscard]] rpl::producer<QString> Events(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);
[[nodiscard]] rpl::producer<QString> EventsWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);
void Set(
	const QString &key,
	QVariant value,
	uint64 accountId = 0,
	bool isTestAccount = false);
void SetAfterRestart(
	const QString &key,
	QVariant value,
	uint64 accountId = 0,
	bool isTestAccount = false);
void Reset(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);
void ResetAfterRestart(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false);

inline bool GetBool(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toBool();
}

inline int GetInt(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toInt();
}

inline QString GetString(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toString();
}

inline QJsonArray GetJsonArray(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toJsonArray();
}

inline bool GetBoolWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toBool();
}

inline int GetIntWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toInt();
}

inline QString GetStringWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toString();
}

inline QJsonArray GetJsonArrayWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toJsonArray();
}

} // namespace JsonSettings

inline bool streamerMode() { return JsonSettings::GetBool("streamer_mode"); }
inline bool hideNotificationsAutomatically() { return JsonSettings::GetBool("auto_hide_notifications"); }
inline bool showUserpicInTopBar() { return JsonSettings::GetBool("userpic_in_top_bar"); }
inline bool connectionBarLost() { return JsonSettings::GetBool("connection_bar_lost"); }
inline bool connectionBarProxy() { return JsonSettings::GetBool("connection_bar_proxy"); }
inline QString appIcon() { return JsonSettings::GetString("app_icon"); }
inline int userpicRoundness() { return JsonSettings::GetInt("userpic_roundness"); }
inline bool generalRoundness() { return JsonSettings::GetBool("general_roundness"); }
inline bool sidebarMyProfile() { return JsonSettings::GetBool("sidebar_my_profile"); }
inline bool sidebarBots() { return JsonSettings::GetBool("sidebar_bots"); }
inline bool sidebarCreateGroup() { return JsonSettings::GetBool("sidebar_create_group"); }
inline bool sidebarCreateChannel() { return JsonSettings::GetBool("sidebar_create_channel"); }
inline bool sidebarContacts() { return JsonSettings::GetBool("sidebar_contacts"); }
inline bool sidebarCalls() { return JsonSettings::GetBool("sidebar_calls"); }
inline bool sidebarSavedMessages() { return JsonSettings::GetBool("sidebar_saved_messages"); }
inline bool sidebarNightMode() { return JsonSettings::GetBool("sidebar_night_mode"); }
inline int stickerSize() { return JsonSettings::GetInt("sticker_size"); }
inline bool showActionsTime() { return JsonSettings::GetBool("show_actions_time"); }
inline bool showSeconds() { return JsonSettings::GetBool("show_seconds"); }
inline bool commaAfterMention() { return JsonSettings::GetBool("comma_after_mention"); }
inline bool hideBubbleTails() { return JsonSettings::GetBool("hide_bubble_tails"); }
inline int stickerShape() { return JsonSettings::GetInt("sticker_shape"); }
inline bool moreRecentStickers() { return JsonSettings::GetBool("more_recent_stickers"); }
inline bool spoofWebviewAsAndroid() { return JsonSettings::GetBool("spoof_webview_as_android"); }

inline void setStreamerMode(bool value) { JsonSettings::Set("streamer_mode", value); }
inline void setHideNotificationsAutomatically(bool value) { JsonSettings::Set("auto_hide_notifications", value); }
inline void setShowUserpicInTopBar(bool value) { JsonSettings::Set("userpic_in_top_bar", value); }
inline void setConnectionBarLost(bool value) { JsonSettings::Set("connection_bar_lost", value); }
inline void setConnectionBarProxy(bool value) { JsonSettings::Set("connection_bar_proxy", value); }
inline void setAppIcon(QString value) { JsonSettings::Set("app_icon", value); }
inline void setUserpicRoundness(int value) { JsonSettings::Set("userpic_roundness", value); }
inline void setGeneralRoundness(bool value) { JsonSettings::Set("general_roundness", value); }
inline void setSidebarMyProfile(bool value) { JsonSettings::Set("sidebar_my_profile", value); }
inline void setSidebarBots(bool value) { JsonSettings::Set("sidebar_bots", value); }
inline void setSidebarCreateGroup(bool value) { JsonSettings::Set("sidebar_create_group", value); }
inline void setSidebarCreateChannel(bool value) { JsonSettings::Set("sidebar_create_channel", value); }
inline void setSidebarContacts(bool value) { JsonSettings::Set("sidebar_contacts", value); }
inline void setSidebarCalls(bool value) { JsonSettings::Set("sidebar_calls", value); }
inline void setSidebarSavedMessages(bool value) { JsonSettings::Set("sidebar_saved_messages", value); }
inline void setSidebarNightMode(bool value) { JsonSettings::Set("sidebar_night_mode", value); }
inline void setStickerSize(int value) { JsonSettings::Set("sticker_size", value); }
inline void setShowActionsTime(bool value) { JsonSettings::Set("show_actions_time", value); }
inline void setShowSeconds(bool value) { JsonSettings::Set("show_seconds", value); }
inline void setCommaAfterMention(bool value) { JsonSettings::Set("comma_after_mention", value); }
inline void setHideBubbleTails(bool value) { JsonSettings::Set("hide_bubble_tails", value); }
inline void setStickerShape(int value) { JsonSettings::Set("sticker_shape", value); }
inline void setMoreRecentStickers(bool value) { JsonSettings::Set("more_recent_stickers", value); }
inline void setSpoofWebviewAsAndroid(bool value) { JsonSettings::Set("spoof_webview_as_android", value); }
	
} // namespace RabbitSettings
