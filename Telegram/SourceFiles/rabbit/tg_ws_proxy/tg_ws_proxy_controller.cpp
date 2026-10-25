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
	data.password = _server.secretHex();
	_currentProxy = data;

	if (Core::IsAppLaunched()) {
		auto &proxySettings = Core::App().settings().proxy();
		if (!_hasSavedState) {
			_savedSettings = proxySettings.settings();
			_savedProxy = proxySettings.selected();
			_hasSavedState = true;
		}

		auto list = proxySettings.list();
		const auto it = std::find_if(list.begin(), list.end(), [&](const MTP::ProxyData &p) {
			return (p.host == data.host) && (p.port == data.port);
		});
		if (it == list.end()) {
			list.push_back(data);
			proxySettings.setList(std::move(list));
		}

		proxySettings.setSelected(data);
		proxySettings.setSettings(MTP::ProxyData::Settings::Enabled);
		Core::App().refreshGlobalProxy();
	}

	_runningChanges.fire(true);
	return true;
}

void Controller::stop() {
	if (!isRunning()) {
		return;
	}

	_server.stop();
	_currentProxy = MTP::ProxyData();

	if (Core::IsAppLaunched() && _hasSavedState) {
		auto &proxySettings = Core::App().settings().proxy();
		proxySettings.setSelected(_savedProxy);
		proxySettings.setSettings(_savedSettings);
		_hasSavedState = false;
		Core::App().refreshGlobalProxy();
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
	return _runningChanges.events_starting_with(isRunning());
}

} // namespace Rabbit::TgWsProxy
