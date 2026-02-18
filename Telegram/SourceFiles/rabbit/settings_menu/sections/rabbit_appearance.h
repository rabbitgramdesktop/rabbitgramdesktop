/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "settings/settings_common.h"
#include "settings/settings_common_session.h"

class BoxContent;

namespace Window
{
    class Controller;

    class SessionController;
} // namespace Window

namespace Settings
{
    class RabbitAppearance : public Section<RabbitAppearance>
    {
    public:
        RabbitAppearance(QWidget* parent, not_null<Window::SessionController*> controller);

        [[nodiscard]] rpl::producer<QString> title() override;

    private:
        void setupContent();
    };
} // namespace Settings
