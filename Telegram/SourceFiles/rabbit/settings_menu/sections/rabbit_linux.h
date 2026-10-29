/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#if !defined Q_OS_WIN && !defined Q_OS_MAC

#include "settings/settings_common.h"
#include "settings/settings_common_session.h"

namespace Window {
class SessionController;
} // namespace Window

namespace Settings {

class RabbitLinux final : public Section<RabbitLinux> {
public:
	RabbitLinux(
		QWidget *parent,
		not_null<Window::SessionController *> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

};

} // namespace Settings

#endif // !defined Q_OS_WIN && !defined Q_OS_MAC
