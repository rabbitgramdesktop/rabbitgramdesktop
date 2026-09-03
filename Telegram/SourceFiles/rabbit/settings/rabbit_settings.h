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
#include <QtCore/QJsonObject>
#include <vector>
#include "rabbit/settings/quick_action.h"

class PeerData;

namespace RabbitSettings {

inline const auto DEFAULT_ICON = u"default"_q;
inline const auto ANGEL_ICON = u"angel"_q;
inline const auto BLACKOUT_ICON = u"blackout"_q;
inline const auto COLOR_ICON = u"color"_q;
inline const auto IMPOSTOR_ICON = u"impostor"_q;
inline const auto MATRIX_ICON = u"matrix"_q;
inline const auto MOON_ICON = u"moon"_q;
inline const auto OLD_ICON = u"old"_q;
inline const auto SEASONAL_ICON = u"seasonal"_q;
inline const auto TWITCH_ICON = u"twitch"_q;

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
	const QVariant &value,
	uint64 accountId = 0,
	bool isTestAccount = false);
void SetAfterRestart(
	const QString &key,
	const QVariant &value,
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

[[nodiscard]] QByteArray ExportSettingsJson();
bool ImportSettingsFromJson(const QByteArray &json);

[[nodiscard]] inline bool GetBool(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toBool();
}

[[nodiscard]] inline int GetInt(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toInt();
}

[[nodiscard]] inline QString GetString(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toString();
}

[[nodiscard]] inline QJsonArray GetJsonArray(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return Get(key, accountId, isTestAccount).toJsonArray();
}

[[nodiscard]] inline bool GetBoolWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toBool();
}

[[nodiscard]] inline int GetIntWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toInt();
}

[[nodiscard]] inline QString GetStringWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toString();
}

[[nodiscard]] inline QJsonArray GetJsonArrayWithPending(
	const QString &key,
	uint64 accountId = 0,
	bool isTestAccount = false) {
	return GetWithPending(key, accountId, isTestAccount).toJsonArray();
}

[[nodiscard]] QJsonObject GetFeaturesTelemetryData();

} // namespace JsonSettings

inline bool streamerMode() { return JsonSettings::GetBool(u"streamer_mode"_q); }
inline bool hideNotificationsAutomatically() { return JsonSettings::GetBool(u"auto_hide_notifications"_q); }
inline bool showUserpicInTopBar() { return JsonSettings::GetBool(u"userpic_in_top_bar"_q); }
inline bool showPeerIdDc() { return JsonSettings::GetBool(u"show_peer_id_dc"_q); }
inline bool connectionBarLost() { return JsonSettings::GetBool(u"connection_bar_lost"_q); }
inline bool connectionBarProxy() { return JsonSettings::GetBool(u"connection_bar_proxy"_q); }
inline QString appIcon() { return JsonSettings::GetString(u"app_icon"_q); }
inline int userpicRoundness() { return JsonSettings::GetInt(u"userpic_roundness"_q); }
inline bool generalRoundness() { return JsonSettings::GetBool(u"general_roundness"_q); }
inline bool sidebarMyProfile() { return JsonSettings::GetBool(u"sidebar_my_profile"_q); }
inline bool sidebarBots() { return JsonSettings::GetBool(u"sidebar_bots"_q); }
inline bool sidebarCreateGroup() { return JsonSettings::GetBool(u"sidebar_create_group"_q); }
inline bool sidebarCreateChannel() { return JsonSettings::GetBool(u"sidebar_create_channel"_q); }
inline bool sidebarContacts() { return JsonSettings::GetBool(u"sidebar_contacts"_q); }
inline bool sidebarCalls() { return JsonSettings::GetBool(u"sidebar_calls"_q); }
inline bool sidebarSavedMessages() { return JsonSettings::GetBool(u"sidebar_saved_messages"_q); }
inline bool sidebarNightMode() { return JsonSettings::GetBool(u"sidebar_night_mode"_q); }
inline int stickerSize() { return JsonSettings::GetInt(u"sticker_size"_q); }
inline int messageRoundness() { return JsonSettings::GetInt(u"message_roundness"_q); }
inline bool showActionsTime() { return JsonSettings::GetBool(u"show_actions_time"_q); }
inline bool showSeconds() { return JsonSettings::GetBool(u"show_seconds"_q); }
inline bool commaAfterMention() { return JsonSettings::GetBool(u"comma_after_mention"_q); }
inline bool hideBubbleTails() { return JsonSettings::GetBool(u"hide_bubble_tails"_q); }
inline int stickerShape() { return JsonSettings::GetInt(u"sticker_shape"_q); }
inline int outgoingQuickAction() { return JsonSettings::GetInt(u"outgoing_quick_action"_q); }
inline int incomingQuickAction() { return JsonSettings::GetInt(u"incoming_quick_action"_q); }
inline std::vector<QuickAction> swipeActionsList() {
	const auto arr = JsonSettings::GetJsonArray(u"swipe_actions_list"_q);
	auto result = std::vector<QuickAction>();
	for (const auto val : arr) {
		result.push_back(static_cast<QuickAction>(val.toInt()));
	}
	if (result.empty()) {
		result.push_back(QuickAction::Reply);
	}
	return result;
}
inline void setSwipeActionsList(const std::vector<QuickAction> &list) {
	auto arr = QJsonArray();
	for (const auto action : list) {
		arr.append(static_cast<int>(action));
	}
	JsonSettings::Set(u"swipe_actions_list"_q, arr);
}
inline bool moreRecentStickers() { return JsonSettings::GetBool(u"more_recent_stickers"_q); }
inline bool spoofWebviewAsAndroid() { return JsonSettings::GetBool(u"spoof_webview_as_android"_q); }
inline bool preciseTime() { return JsonSettings::GetBool(u"precise_time"_q); }
inline bool showOnlineIndicator() { return JsonSettings::GetBool(u"show_online_indicator"_q); }
inline int quietModeDeadline() { return JsonSettings::GetInt(u"quiet_mode_deadline"_q); }
inline bool quietModeEnabled() { return JsonSettings::GetBool(u"quiet_mode_enabled"_q); }
inline bool telemetryAppVersion() { return JsonSettings::GetBool(u"telemetry_app_version"_q); }
inline bool telemetryOs() { return JsonSettings::GetBool(u"telemetry_os"_q); }
inline bool telemetryFeatures() { return JsonSettings::GetBool(u"telemetry_features"_q); }
inline bool telemetryCpuArch() { return JsonSettings::GetBool(u"telemetry_cpu_arch"_q); }
inline bool analytics() { return JsonSettings::GetBool(u"analytics"_q); }
inline QString telemetryClientId() { return JsonSettings::GetString(u"telemetry_client_id"_q); }
inline bool replaceEditedLabel() { return JsonSettings::GetBool(u"replace_edited_label"_q); }
inline bool hideReactionsChannels() { return JsonSettings::GetBool(u"hide_reactions_channels"_q); }
inline bool hideReactionsGroups() { return JsonSettings::GetBool(u"hide_reactions_groups"_q); }
inline bool hideReactionsPrivates() { return JsonSettings::GetBool(u"hide_reactions_privates"_q); }
inline bool hideReactions() { return hideReactionsChannels() || hideReactionsGroups() || hideReactionsPrivates(); }

inline void setStreamerMode(bool value) { JsonSettings::Set(u"streamer_mode"_q, value); }
inline void setHideNotificationsAutomatically(bool value) { JsonSettings::Set(u"auto_hide_notifications"_q, value); }
inline void setShowUserpicInTopBar(bool value) { JsonSettings::Set(u"userpic_in_top_bar"_q, value); }
inline void setShowPeerIdDc(bool value) { JsonSettings::Set(u"show_peer_id_dc"_q, value); }
inline void setConnectionBarLost(bool value) { JsonSettings::Set(u"connection_bar_lost"_q, value); }
inline void setConnectionBarProxy(bool value) { JsonSettings::Set(u"connection_bar_proxy"_q, value); }
inline void setAppIcon(const QString &value) { JsonSettings::Set(u"app_icon"_q, value); }
inline void setUserpicRoundness(int value) { JsonSettings::Set(u"userpic_roundness"_q, value); }
inline void setGeneralRoundness(bool value) { JsonSettings::Set(u"general_roundness"_q, value); }
inline void setSidebarMyProfile(bool value) { JsonSettings::Set(u"sidebar_my_profile"_q, value); }
inline void setSidebarBots(bool value) { JsonSettings::Set(u"sidebar_bots"_q, value); }
inline void setSidebarCreateGroup(bool value) { JsonSettings::Set(u"sidebar_create_group"_q, value); }
inline void setSidebarCreateChannel(bool value) { JsonSettings::Set(u"sidebar_create_channel"_q, value); }
inline void setSidebarContacts(bool value) { JsonSettings::Set(u"sidebar_contacts"_q, value); }
inline void setSidebarCalls(bool value) { JsonSettings::Set(u"sidebar_calls"_q, value); }
inline void setSidebarSavedMessages(bool value) { JsonSettings::Set(u"sidebar_saved_messages"_q, value); }
inline void setSidebarNightMode(bool value) { JsonSettings::Set(u"sidebar_night_mode"_q, value); }
inline void setStickerSize(int value) { JsonSettings::Set(u"sticker_size"_q, value); }
inline void setMessageRoundness(int value) { JsonSettings::Set(u"message_roundness"_q, value); }
inline void setShowActionsTime(bool value) { JsonSettings::Set(u"show_actions_time"_q, value); }
inline void setShowSeconds(bool value) { JsonSettings::Set(u"show_seconds"_q, value); }
inline void setCommaAfterMention(bool value) { JsonSettings::Set(u"comma_after_mention"_q, value); }
inline void setHideBubbleTails(bool value) { JsonSettings::Set(u"hide_bubble_tails"_q, value); }
inline void setStickerShape(int value) { JsonSettings::Set(u"sticker_shape"_q, value); }
inline void setOutgoingQuickAction(int value) { JsonSettings::Set(u"outgoing_quick_action"_q, value); }
inline void setIncomingQuickAction(int value) { JsonSettings::Set(u"incoming_quick_action"_q, value); }
inline void setMoreRecentStickers(bool value) { JsonSettings::Set(u"more_recent_stickers"_q, value); }
inline void setSpoofWebviewAsAndroid(bool value) { JsonSettings::Set(u"spoof_webview_as_android"_q, value); }
inline void setPreciseTime(bool value) { JsonSettings::Set(u"precise_time"_q, value); }
inline void setShowOnlineIndicator(bool value) { JsonSettings::Set(u"show_online_indicator"_q, value); }
inline void setQuietModeDeadline(int value) { JsonSettings::Set(u"quiet_mode_deadline"_q, value); }
inline void setQuietModeEnabled(bool value) {
	JsonSettings::Set(u"quiet_mode_enabled"_q, value);
	if (!value) {
		setQuietModeDeadline(0);
	}
}
inline void setTelemetryAppVersion(bool value) { JsonSettings::Set(u"telemetry_app_version"_q, value); }
inline void setTelemetryOs(bool value) { JsonSettings::Set(u"telemetry_os"_q, value); }
inline void setTelemetryFeatures(bool value) { JsonSettings::Set(u"telemetry_features"_q, value); }
inline void setTelemetryCpuArch(bool value) { JsonSettings::Set(u"telemetry_cpu_arch"_q, value); }
inline void setAnalytics(bool value) { JsonSettings::Set(u"analytics"_q, value); }
inline void setTelemetryClientId(const QString &value) { JsonSettings::Set(u"telemetry_client_id"_q, value); }
inline void setReplaceEditedLabel(bool value) { JsonSettings::Set(u"replace_edited_label"_q, value); }
inline void setHideReactions(bool value) { JsonSettings::Set(u"hide_reactions"_q, value); }
inline void setHideReactionsChannels(bool value) { JsonSettings::Set(u"hide_reactions_channels"_q, value); }
inline void setHideReactionsGroups(bool value) { JsonSettings::Set(u"hide_reactions_groups"_q, value); }
inline void setHideReactionsPrivates(bool value) { JsonSettings::Set(u"hide_reactions_privates"_q, value); }

[[nodiscard]] bool ShouldHideReactions(const PeerData *peer);

} // namespace RabbitSettings
