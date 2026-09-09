/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/swipe_actions/swipe_action.h"
#include "data/data_peer.h"

#include "base/parse_helper.h"
#include "base/timer.h"
#include "core/application.h"
#include "core/version.h"
#include "mainwindow.h"
#include "mainwidget.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QTimer>
#include <QtCore/QSet>

namespace RabbitSettings::JsonSettings {
namespace {

constexpr auto kWriteJsonTimeout = static_cast<crl::time>(5000);

class Manager : public QObject {
public:
	Manager();
	void load();
	void fill();
	void write(bool force = false);

	[[nodiscard]] QVariant get(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false);
	[[nodiscard]] QVariant getWithPending(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false);
	[[nodiscard]] QVariantMap getAllWithPending(const QString &key) const;
	[[nodiscard]] rpl::producer<QString> events(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false) const;
	[[nodiscard]] rpl::producer<QString> eventsWithPending(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false) const;
	void set(
		const QString &key,
		const QVariant &value,
		uint64 accountId = 0,
		bool isTestAccount = false);
	void setAfterRestart(
		const QString &key,
		const QVariant &value,
		uint64 accountId = 0,
		bool isTestAccount = false);
	void reset(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false);
	void resetAfterRestart(
		const QString &key,
		uint64 accountId = 0,
		bool isTestAccount = false);
	void writeTimeout();

private:
	[[nodiscard]] static QVariant getDefault(const QString &key);

	static void writeDefaultFile();
	void writeCurrentSettings();
	bool readCustomFile();
	void writing();

	base::Timer _jsonWriteTimer;

	rpl::event_stream<QString> _eventStream;
	rpl::event_stream<QString> _pendingEventStream;
	QHash<QString, QVariant> _settingsHashMap;
	QHash<QString, QVariant> _defaultSettingsHashMap;

};

QString MakeMapKey(const QString &key, uint64 accountId, bool isTestAccount) {
	return (accountId == 0)
		? key
		: (key
			+ (isTestAccount ? u":test_"_q : u":"_q)
			+ QString::number(accountId));
}

QVariantMap GetAllWithPending(const QString &key);

enum SettingScope {
	Global,
	Account,
};

enum SettingStorage {
	None,
	MainJson,
};

enum SettingType {
	BoolSetting,
	IntSetting,
	QStringSetting,
	QJsonArraySetting,
};

using CheckHandler = Fn<QVariant(QVariant)>;

CheckHandler IntLimit(int min, int max, int defaultValue) {
	return [=](QVariant value) -> QVariant {
		if (!value.canConvert<int>()) {
			return defaultValue;
		}
		const auto intVal = value.toInt();
		return (intVal < min) ? min : (intVal > max) ? max : value;
	};
}

CheckHandler IntLimit(int min, int max) {
	return IntLimit(min, max, min);
}

CheckHandler IntLimitMin(int min) {
	return [=](QVariant value) -> QVariant {
		if (!value.canConvert<int>()) {
			return min;
		}
		const auto intVal = value.toInt();
		return (intVal < min) ? min : value;
	};
}

CheckHandler ScalesLimit() {
	return [=](const QVariant &value) -> QVariant {
		auto newArrayValue = QJsonArray();
		if (value.canConvert<QJsonArray>()) {
			const auto arrayValue = value.toJsonArray();
			for (auto i = arrayValue.begin(); i != arrayValue.end() && arrayValue.size() <= 6; ++i) {
				const auto scaleNumber = i->toDouble();
				if (scaleNumber >= style::kScaleMin && scaleNumber <= style::kScaleMax) {
					newArrayValue.append(scaleNumber);
				}
			}
		}
		return newArrayValue;
	};
}

struct Definition {
	SettingScope scope = Global;
	SettingStorage storage = MainJson;
	SettingType type = BoolSetting;
	QVariant defaultValue;
	QVariant fillerValue;
	CheckHandler limitHandler = nullptr;
};

QJsonValue GetObjectValue(const QJsonObject &settings, QStringList &keyParts) {
	if (keyParts.isEmpty()) {
		return QJsonValue();
	}
	const auto firstKey = keyParts.takeFirst();
	if (!settings.contains(firstKey)) {
		return QJsonValue();
	}
	auto resultRef = settings.value(firstKey);
	for (const auto &key : keyParts) {
		const auto referenced = resultRef.toObject();
		if (!referenced.contains(key)) {
			return QJsonValue();
		}
		resultRef = referenced.value(key);
	}
	return resultRef;
}

QVariant ParseJsonValue(const QJsonValue &val, const Definition &def) {
	switch (def.type) {
	case BoolSetting:
		if (val.isBool()) {
			return val.toBool();
		} else if (val.isDouble()) {
			return val.toDouble() != 0.0;
		}
		break;
	case IntSetting:
		if (val.isDouble()) {
			const auto intValue = qFloor(val.toDouble());
			return def.limitHandler ? def.limitHandler(intValue) : intValue;
		}
		break;
	case QStringSetting:
		if (val.isString()) {
			return val.toString();
		}
		break;
	case QJsonArraySetting:
		if (val.isArray()) {
			const auto arrayValue = val.toArray();
			return def.limitHandler ? def.limitHandler(arrayValue) : arrayValue;
		}
		break;
	}
	return QVariant();
}

const std::map<QString, Definition, std::greater<QString>> DefinitionMap{
	{
		u"api_id"_q, {
			.storage = None,
			.type = IntSetting,
#if defined TDESKTOP_API_ID && defined TDESKTOP_API_HASH
			.defaultValue = TDESKTOP_API_ID,
#else
			.defaultValue = 0,
#endif
		}
	},
	{
		u"api_hash"_q, {
			.storage = None,
			.type = QStringSetting,
#if defined TDESKTOP_API_ID && defined TDESKTOP_API_HASH
			.defaultValue = QT_STRINGIFY(TDESKTOP_API_HASH),
#else
			.defaultValue = QString(),
#endif
		}
	},
	{
		u"api_use_env"_q, {
			.storage = None,
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"api_start_params"_q, {
			.storage = None,
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"streamer_mode"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"math_autocomplete"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"auto_hide_notifications"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"userpic_in_top_bar"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"show_peer_id_dc"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"connection_bar_lost"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"connection_bar_proxy"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"app_icon"_q, {
			.type = QStringSetting,
			.defaultValue = DEFAULT_ICON,
		}
	},
	{
		u"userpic_roundness"_q, {
			.type = IntSetting,
			.defaultValue = 50,
			.limitHandler = IntLimit(0, 50, 50),
		}
	},
	{
		u"general_roundness"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"sidebar_my_profile"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_bots"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_create_group"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_create_channel"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_contacts"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_calls"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_saved_messages"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sidebar_night_mode"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"sticker_size"_q, {
			.type = IntSetting,
			.defaultValue = 170,
			.limitHandler = IntLimit(64, 256, 170),
		}
	},
	{
		u"message_roundness"_q, {
			.type = IntSetting,
			.defaultValue = 50,
			.limitHandler = IntLimit(0, 50, 50),
		}
	},
	{
		u"show_actions_time"_q, {
			.type = BoolSetting,
			.defaultValue = false
		}
	},
	{
		u"show_seconds"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"comma_after_mention"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"hide_bubble_tails"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"sticker_shape"_q, {
			.type = IntSetting,
			.defaultValue = 0,
			.limitHandler = IntLimit(0, 2, 0),
		}
	},
	{
		u"outgoing_quick_action"_q, {
			.type = IntSetting,
			.defaultValue = 1,
			.limitHandler = IntLimit(0, 7, 1),
		}
	},
	{
		u"incoming_quick_action"_q, {
			.type = IntSetting,
			.defaultValue = 1,
			.limitHandler = IntLimit(0, 7, 1),
		}
	},
	{
		u"swipe_actions_list"_q, {
			.type = QJsonArraySetting,
			.defaultValue = QJsonArray{ static_cast<int>(SwipeAction::Reply) },
		}
	},
	{
		u"more_recent_stickers"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"spoof_webview_as_android"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"precise_time"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"show_online_indicator"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"quiet_mode_deadline"_q, {
			.type = IntSetting,
			.defaultValue = 0,
		}
	},
	{
		u"quiet_mode_enabled"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"telemetry_app_version"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"telemetry_os"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"telemetry_features"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"telemetry_cpu_arch"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"analytics"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"telemetry_client_id"_q, {
			.type = QStringSetting,
			.defaultValue = QString(),
		}
	},
	{
		u"hide_reactions"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"hide_reactions_channels"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"hide_reactions_groups"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"channel_bottom_button"_q, {
			.type = IntSetting,
			.defaultValue = 0,
		}
	},
	{
		u"replace_edited_label"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"hide_reactions_privates"_q, {
			.type = BoolSetting,
			.defaultValue = false,
		}
	},
	{
		u"reply_colors"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"reply_emoji"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"reply_bg"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"quick_swipe_channels"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	},
	{
		u"quick_swipe_topics"_q, {
			.type = BoolSetting,
			.defaultValue = true,
		}
	}
};

using OldOptionKey = QString;
using NewOptionKey = QString;

const std::map<OldOptionKey, NewOptionKey, std::greater<OldOptionKey>> ReplacedOptionsMap{

};

QString DefaultFilePath() {
	return cWorkingDir() + u"tdata/rabbit-settings-default.json"_q;
}

QString CustomFilePath() {
	return cWorkingDir() + u"tdata/rabbit-settings-custom.json"_q;
}

bool DefaultFileIsValid() {
	QFile file(DefaultFilePath());
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	auto error = QJsonParseError{ 0, QJsonParseError::NoError };
	const auto document = QJsonDocument::fromJson(
		base::parse::stripComments(file.readAll()),
		&error);
	file.close();

	if (error.error != QJsonParseError::NoError || !document.isObject()) {
		return false;
	}
	const auto settings = document.object();

	const auto version = settings.constFind(u"version"_q);
	if (version == settings.constEnd() || version->toInt() != AppVersion) {
		return false;
	}

	return true;
}

void WriteDefaultCustomFile() {
	const auto path = CustomFilePath();
	auto input = QFile(u":/misc/default_rabbit-settings-custom.json"_q);
	auto output = QFile(path);
	if (input.open(QIODevice::ReadOnly) && output.open(QIODevice::WriteOnly)) {
		output.write(input.readAll());
	}
}

QByteArray GenerateSettingsJson(bool areDefault = false) {
	auto settings = QJsonObject();

	const auto getRef = [&settings](
		QStringList &keyParts,
		const Definition &def) -> QJsonValueRef {
		const auto firstKey = keyParts.takeFirst();
		if (!settings.contains(firstKey)) {
			settings.insert(firstKey, QJsonObject());
		}
		auto resultRef = settings[firstKey];
		for (const auto &key : keyParts) {
			auto referenced = resultRef.toObject();
			if (!referenced.contains(key)) {
				referenced.insert(key, QJsonObject());
				resultRef = referenced;
			}
			resultRef = referenced[key];
		}
		return resultRef;
	};

	const auto getValue = [=](
		const QString &key,
		const Definition &def) -> QJsonValue {
		auto value = (!areDefault)
			? GetWithPending(key)
			: def.fillerValue.isValid()
			? def.fillerValue
			: def.defaultValue.isValid()
			? def.defaultValue
			: QVariant();
		switch (def.type) {
		case BoolSetting:
			return value.isValid() ? value.toBool() : false;
		case IntSetting:
			return value.isValid() ? value.toInt() : 0;
		case QStringSetting:
			return value.isValid() ? value.toString() : QString();
		case QJsonArraySetting:
			return value.isValid() ? value.toJsonArray() : QJsonArray();
		}

		return QJsonValue();
	};

	const auto getAccountValue = [=](const QString &key) -> QJsonValue {
		if (areDefault) {
			return QJsonObject();
		}

		const auto values = GetAllWithPending(key);
		auto resultObject = QJsonObject();

		for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
			const auto value = i.value();
			const auto jsonValue = (value.userType() == QMetaType::Bool)
				? QJsonValue(value.toBool())
				: (value.userType() == QMetaType::Int)
				? QJsonValue(value.toInt())
				: (value.userType() == QMetaType::QString)
				? QJsonValue(value.toString())
				: (value.userType() == QMetaType::QJsonArray)
				? QJsonValue(value.toJsonArray())
				: QJsonValue(QJsonValue::Null);
			resultObject.insert(i.key(), jsonValue);
		}

		return resultObject;
	};

	for (const auto &[key, def] : DefinitionMap) {
		if (def.storage == None) {
			continue;
		}

		auto parts = key.split(QChar('/'));
		auto value = (def.scope == Account)
			? getAccountValue(key)
			: getValue(key, def);
		if (parts.size() > 1) {
			const auto lastKey = parts.takeLast();
			auto ref = getRef(parts, def);
			auto referenced = ref.toObject();
			referenced.insert(lastKey, value);
			ref = referenced;
		} else {
			settings.insert(key, value);
		}
	}

	if (areDefault) {
		settings.insert(u"version"_q, QString::number(AppVersion));
	}

	auto document = QJsonDocument();
	document.setObject(settings);
	return document.toJson(QJsonDocument::Indented);
}

std::unique_ptr<Manager> Data;

QVariantMap GetAllWithPending(const QString &key) {
	return Data ? Data->getAllWithPending(key) : QVariantMap();
}

} // namespace

Manager::Manager()
	: _jsonWriteTimer([=] { writeTimeout(); }) {
}

void Manager::load() {
	if (!DefaultFileIsValid()) {
		writeDefaultFile();
	}
	if (!readCustomFile()) {
		WriteDefaultCustomFile();
	}
}

void Manager::fill() {
	_settingsHashMap.reserve(DefinitionMap.size());
	_defaultSettingsHashMap.reserve(DefinitionMap.size());

	const auto addDefaultValue = [&](const QString &option, const QVariant &value) {
		_settingsHashMap.insert(option, value);
	};

	for (const auto &[key, def] : DefinitionMap) {
		if (def.scope != Global) {
			continue;
		}

		auto defaultValue = def.defaultValue;
		if (!defaultValue.isValid()) {
			if (def.type == BoolSetting) {
				defaultValue = false;
			} else if (def.type == IntSetting) {
				defaultValue = 0;
			} else if (def.type == QStringSetting) {
				defaultValue = QString();
			} else if (def.type == QJsonArraySetting) {
				defaultValue = QJsonArray();
			} else {
				continue;
			}
		}

		addDefaultValue(key, defaultValue);
	}
}

void Manager::write(bool force) {
	if (force && _jsonWriteTimer.isActive()) {
		_jsonWriteTimer.cancel();
		writeTimeout();
	} else if (!force && !_jsonWriteTimer.isActive()) {
		_jsonWriteTimer.callOnce(kWriteJsonTimeout);
	}
}

QVariant Manager::get(const QString &key, uint64 accountId, bool isTestAccount) {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	if (const auto it = _settingsHashMap.constFind(mapKey); it != _settingsHashMap.constEnd() && it.value().isValid()) {
		return it.value();
	}
	auto result = _settingsHashMap.contains(key)
		? _settingsHashMap.value(key)
		: getDefault(key);
	_settingsHashMap.insert(mapKey, result);
	return result;
}

QVariant Manager::getWithPending(const QString &key, uint64 accountId, bool isTestAccount) {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	if (const auto it = _defaultSettingsHashMap.constFind(mapKey); it != _defaultSettingsHashMap.constEnd() && it.value().isValid()) {
		return it.value();
	}
	if (const auto it = _settingsHashMap.constFind(mapKey); it != _settingsHashMap.constEnd() && it.value().isValid()) {
		return it.value();
	}
	auto result = _settingsHashMap.contains(key)
		? _settingsHashMap.value(key)
		: getDefault(key);
	_settingsHashMap.insert(mapKey, result);
	return result;
}

QVariantMap Manager::getAllWithPending(const QString &key) const {
	auto resultMap = QVariantMap();

	if (const auto it = _defaultSettingsHashMap.constFind(key); it != _defaultSettingsHashMap.constEnd()) {
		resultMap.insert(u"0"_q, it.value());
		return resultMap;
	}
	if (const auto it = _settingsHashMap.constFind(key); it != _settingsHashMap.constEnd()) {
		resultMap.insert(u"0"_q, it.value());
		return resultMap;
	}

	const auto prefix = key + u":"_q;

	for (auto i = _settingsHashMap.constBegin(); i != _settingsHashMap.constEnd(); ++i) {
		const auto mapKey = i.key();
		if (mapKey.startsWith(prefix)) {
			resultMap.insert(mapKey.mid(prefix.size()), i.value());
		}
	}

	for (auto i = _defaultSettingsHashMap.constBegin(); i != _defaultSettingsHashMap.constEnd(); ++i) {
		const auto mapKey = i.key();
		if (mapKey.startsWith(prefix)) {
			resultMap.insert(mapKey.mid(prefix.size()), i.value());
		}
	}

	return resultMap;
}

QVariant Manager::getDefault(const QString &key) {
	const auto &defIterator = DefinitionMap.find(key);
	if (defIterator == DefinitionMap.end()) {
		return QVariant();
	}
	const auto defaultValue = &defIterator->second.defaultValue;
	switch (defIterator->second.type) {
	case QStringSetting:
		return QVariant(defaultValue->isValid()
			? defaultValue->toString()
			: QString());
	case IntSetting:
		return QVariant(defaultValue->isValid()
			? defaultValue->toInt()
			: 0);
	case BoolSetting:
		return QVariant(defaultValue->isValid()
			? defaultValue->toBool()
			: false);
	case QJsonArraySetting:
		return QVariant(defaultValue->isValid()
			? defaultValue->toJsonArray()
			: QJsonArray());
	}

	return QVariant();
}

rpl::producer<QString> Manager::events(const QString &key, uint64 accountId, bool isTestAccount) const {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	return _eventStream.events() | rpl::filter(rpl::mappers::_1 == mapKey);
}

rpl::producer<QString> Manager::eventsWithPending(const QString &key, uint64 accountId, bool isTestAccount) const {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	return _pendingEventStream.events() | rpl::filter(rpl::mappers::_1 == mapKey);
}

void Manager::set(const QString &key, const QVariant &value, uint64 accountId, bool isTestAccount) {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	_settingsHashMap.insert(mapKey, value);
	_eventStream.fire_copy(mapKey);
}

void Manager::setAfterRestart(const QString &key, const QVariant &value, uint64 accountId, bool isTestAccount) {
	const auto mapKey = MakeMapKey(key, accountId, isTestAccount);
	const auto it = _settingsHashMap.constFind(mapKey);
	if (it == _settingsHashMap.constEnd() || it.value() != value) {
		_defaultSettingsHashMap.insert(mapKey, value);
	} else {
		_defaultSettingsHashMap.remove(mapKey);
	}
	_pendingEventStream.fire_copy(mapKey);
}

void Manager::reset(const QString &key, uint64 accountId, bool isTestAccount) {
	set(key, getDefault(key), accountId, isTestAccount);
}

void Manager::resetAfterRestart(const QString &key, uint64 accountId, bool isTestAccount) {
	setAfterRestart(key, getDefault(key), accountId, isTestAccount);
}

bool Manager::readCustomFile() {
	QFile file(CustomFilePath());
	if (!file.exists()) {
		return false;
	}
	if (!file.open(QIODevice::ReadOnly)) {
		return true;
	}
	auto error = QJsonParseError{ 0, QJsonParseError::NoError };
	const auto document = QJsonDocument::fromJson(
		base::parse::stripComments(file.readAll()),
		&error);
	file.close();

	if (error.error != QJsonParseError::NoError || !document.isObject()) {
		return true;
	}
	const auto settings = document.object();

	if (settings.isEmpty()) {
		return true;
	}

	const auto prepareAccountOptions = [](
		const QString &key,
		const Definition &def,
		const QJsonValue &val,
		const Fn<void(const QString &,
			const Definition &,
			const QJsonValue &,
			uint64,
			bool)> &callback) {
		if (val.isUndefined()) {
			return;
		}
		if (def.scope == Account && val.isObject()) {
			const auto accounts = val.toObject();
			if (accounts.isEmpty()) {
				return;
			}

			for (auto i = accounts.constBegin(); i != accounts.constEnd(); ++i) {
				auto optionKey = i.key();
				auto isTestAccount = false;
				if (optionKey.startsWith(u"test_"_q)) {
					isTestAccount = true;
					optionKey = optionKey.mid(5);
				}
				const auto accountId = optionKey.toULongLong();
				callback(key, def, i.value(), accountId, (accountId == 0) ? false : isTestAccount);
			}
		} else {
			callback(key, def, val, 0, false);
		}
	};

	const auto setValue = [this](
		const QString &key,
		const Definition &def,
		const QJsonValue &val,
		uint64 accountId,
		bool isTestAccount) {
		const auto parsed = ParseJsonValue(val, def);
		if (parsed.isValid()) {
			set(key, parsed, accountId, isTestAccount);
		}
	};

	for (const auto &[oldkey, newkey] : ReplacedOptionsMap) {
		const auto &defIterator = DefinitionMap.find(newkey);
		if (defIterator == DefinitionMap.end()) {
			continue;
		}
		auto parts = oldkey.split(QChar('/'));
		const auto val = (parts.size() > 1)
			? GetObjectValue(settings, parts)
			: settings.value(oldkey);

		if (!val.isUndefined()) {
			prepareAccountOptions(newkey, defIterator->second, val, setValue);
		}
	}

	for (const auto &[key, def] : DefinitionMap) {
		if (def.storage == None) {
			continue;
		}
		auto parts = key.split(QChar('/'));
		const auto val = (parts.size() > 1)
			? GetObjectValue(settings, parts)
			: settings.value(key);

		if (!val.isUndefined()) {
			prepareAccountOptions(key, def, val, setValue);
		}
	}
	return true;
}

void Manager::writeDefaultFile() {
	auto file = QFile(DefaultFilePath());
	if (!file.open(QIODevice::WriteOnly)) {
		return;
	}
	file.write(GenerateSettingsJson(true));
}

void Manager::writeCurrentSettings() {
	auto file = QFile(CustomFilePath());
	if (!file.open(QIODevice::WriteOnly)) {
		return;
	}
	if (_jsonWriteTimer.isActive()) {
		writing();
	}
	file.write(GenerateSettingsJson());
}

void Manager::writeTimeout() {
	writeCurrentSettings();
}

void Manager::writing() {
	_jsonWriteTimer.cancel();
}

void Start() {
	if (Data) {
		return;
	}
	Data = std::make_unique<Manager>();
	Data->fill();
}

void Load() {
	if (!Data) {
		return;
	}
	Data->load();
}

void Write() {
	if (!Data) {
		return;
	}
	Data->write();
}

void Finish() {
	if (!Data) {
		return;
	}
	Data->write(true);
}

QVariant Get(const QString &key, uint64 accountId, bool isTestAccount) {
	return Data ? Data->get(key, accountId, isTestAccount) : QVariant();
}

QVariant GetWithPending(const QString &key, uint64 accountId, bool isTestAccount) {
	return Data ? Data->getWithPending(key, accountId, isTestAccount) : QVariant();
}

rpl::producer<QString> Events(const QString &key, uint64 accountId, bool isTestAccount) {
	return Data ? Data->events(key, accountId, isTestAccount) : rpl::single(QString());
}

rpl::producer<QString> EventsWithPending(const QString &key, uint64 accountId, bool isTestAccount) {
	return Data ? Data->eventsWithPending(key, accountId, isTestAccount) : rpl::single(QString());
}

void Set(const QString &key, const QVariant &value, uint64 accountId, bool isTestAccount) {
	if (!Data) {
		return;
	}
	Data->set(key, value, accountId, isTestAccount);
	Write();
}

void SetAfterRestart(const QString &key, const QVariant &value, uint64 accountId, bool isTestAccount) {
	if (!Data) {
		return;
	}
	Data->setAfterRestart(key, value, accountId, isTestAccount);
	Write();
}

void Reset(const QString &key, uint64 accountId, bool isTestAccount) {
	if (!Data) {
		return;
	}
	Data->reset(key, accountId, isTestAccount);
	Write();
}

void ResetAfterRestart(const QString &key, uint64 accountId, bool isTestAccount) {
	if (!Data) {
		return;
	}
	Data->resetAfterRestart(key, accountId, isTestAccount);
	Write();
}

QByteArray ExportSettingsJson() {
	return GenerateSettingsJson();
}

bool ImportSettingsFromJson(const QByteArray &json) {
	auto error = QJsonParseError{ 0, QJsonParseError::NoError };
	const auto document = QJsonDocument::fromJson(json, &error);
	if (error.error != QJsonParseError::NoError || !document.isObject()) {
		return false;
	}
	const auto settings = document.object();
	if (settings.isEmpty()) {
		return false;
	}

	const auto getObjectValue = [&settings](
		QStringList &keyParts,
		const Definition &def) -> QJsonValue {
		const auto firstKey = keyParts.takeFirst();
		if (!settings.contains(firstKey)) {
			return QJsonValue();
		}
		auto resultRef = settings.value(firstKey);
		for (const auto &key : keyParts) {
			auto referenced = resultRef.toObject();
			if (!referenced.contains(key)) {
				return QJsonValue();
			}
			resultRef = referenced.value(key);
		}
		return resultRef;
	};

	for (const auto &[key, def] : DefinitionMap) {
		if (def.storage == None) {
			continue;
		}
		auto parts = key.split(QChar('/'));
		const auto val = (parts.size() > 1)
			? GetObjectValue(settings, parts)
			: settings.value(key);
		if (val.isUndefined()) {
			continue;
		}
		const auto parsed = ParseJsonValue(val, def);
		if (parsed.isValid()) {
			Set(key, parsed);
		}
	}
	return true;
}

QJsonObject GetFeaturesTelemetryData() {
	auto result = QJsonObject();
	const auto blacklist = QSet<QString>{
		u"api_id"_q,
		u"api_hash"_q,
		u"telemetry_client_id"_q,
		u"quiet_mode_deadline"_q,
	};
	for (const auto &[key, def] : DefinitionMap) {
		if (blacklist.contains(key)) {
			continue;
		}
		switch (def.type) {
		case BoolSetting:
			result.insert(key, GetBool(key));
			break;
		case IntSetting:
			result.insert(key, GetInt(key));
			break;
		case QStringSetting:
			result.insert(key, GetString(key));
			break;
		case QJsonArraySetting:
			result.insert(key, GetJsonArray(key));
			break;
		}
	}
	return result;
}

} // namespace RabbitSettings::JsonSettings

namespace RabbitSettings {

bool ShouldHideReactions(const PeerData *peer) {
	if (!peer) {
		return false;
	}
	if (peer->isUser()) {
		return hideReactionsPrivates();
	}
	if (peer->isChat() || (peer->isChannel() && peer->isMegagroup())) {
		return hideReactionsGroups();
	}
	if (peer->isChannel() && !peer->isMegagroup()) {
		return hideReactionsChannels();
	}
	return false;
}

} // namespace RabbitSettings
