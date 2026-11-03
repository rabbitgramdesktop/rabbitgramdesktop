/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/tg_ws_proxy/tg_ws_proxy_controller.h"
#include "rabbit/settings/rabbit_settings.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "main/main_domain.h"
#include "main/main_account.h"
#include "mtproto/mtp_instance.h"

#include <algorithm>

namespace Rabbit::TgWsProxy {

Controller &Controller::Instance() {
	static auto instance = new Controller();
	return *instance;
}

Controller::Controller() {
}

bool Controller::start() {
	if (isRunning()) {
		return true;
	}

	if (!_server.start()) {
		return false;
	}

	auto data = MTP::ProxyData();
	data.type = MTP::ProxyData::Type::Mtproto;
	data.host = u"127.0.0.1"_q;
	data.port = _server.port();
	data.password = u"dd"_q + _server.secretHex();
	_currentProxy = data;

	if (Core::IsAppLaunched()) {
		auto &proxySettings = Core::App().settings().proxy();
		if (!_hasSavedState) {
			const auto &selected = proxySettings.selected();
			if (selected.valid()
				&& selected != data
				&& (selected.host != data.host || selected.port != data.port)) {
				_savedSettings = proxySettings.settings();
				_savedProxy = selected;
			} else {
				_savedSettings = MTP::ProxyData::Settings::Disabled;
				_savedProxy = MTP::ProxyData();
			}
			_hasSavedState = true;
		}

		auto list = proxySettings.list();
		const auto it = std::find_if(list.begin(), list.end(), [&](const MTP::ProxyData &p) {
			return (p.host == data.host) && (p.port == data.port);
		});
		if (it == list.end()) {
			list.push_back(data);
		} else {
			*it = data;
		}
		proxySettings.setList(std::move(list));

		Core::App().setCurrentProxy(data, MTP::ProxyData::Settings::Enabled);
		Core::App().saveSettingsDelayed();
		if (Core::App().domain().started()) {
			for (const auto &[index, account] : Core::App().domain().accounts()) {
				account->mtp().restart();
				account->mtp().reInitConnection(account->mtp().mainDcId());
			}
		}
	}

	_runningChanges.fire(true);
	return true;
}

void Controller::stop() {
	if (!isRunning()) {
		return;
	}

	const auto proxyToRemove = _currentProxy;
	_server.stop();
	_currentProxy = MTP::ProxyData();

	if (Core::IsAppLaunched()) {
		auto &proxySettings = Core::App().settings().proxy();
		if (proxyToRemove.valid()) {
			while (true) {
				const auto &list = proxySettings.list();
				const auto it = std::find_if(list.begin(), list.end(), [&](const MTP::ProxyData &p) {
					return (p.host == proxyToRemove.host) && (p.port == proxyToRemove.port);
				});
				if (it == list.end() || !proxySettings.removeFromList(*it)) {
					break;
				}
			}
		}

		if (_hasSavedState) {
			const auto savedProxy = _savedProxy;
			const auto savedSettings = _savedSettings;
			_hasSavedState = false;
			_savedProxy = MTP::ProxyData();
			_savedSettings = MTP::ProxyData::Settings::Disabled;
			Core::App().setCurrentProxy(savedProxy, savedSettings);
		} else {
			Core::App().setCurrentProxy(
				MTP::ProxyData(),
				MTP::ProxyData::Settings::Disabled);
		}
		Core::App().saveSettingsDelayed();
		if (Core::App().domain().started()) {
			for (const auto &[index, account] : Core::App().domain().accounts()) {
				account->mtp().restart();
				account->mtp().reInitConnection(account->mtp().mainDcId());
			}
		}
	}

	_runningChanges.fire(false);
}

void Controller::applyIfEnabled() {
	if (RabbitSettings::tgWsProxyEnabled()) {
		start();
	} else if (isRunning()) {
		stop();
	}
}

bool Controller::isRunning() const {
	return _server.isListening();
}

MTP::ProxyData Controller::proxyData() const {
	return _currentProxy;
}

rpl::producer<bool> Controller::runningValue() const {
	return _runningChanges.events_starting_with_copy(isRunning());
}

} // namespace Rabbit::TgWsProxy
