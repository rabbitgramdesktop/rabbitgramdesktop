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
    class RabbitChats : public Section<RabbitChats>
    {
    public:
        RabbitChats(QWidget* parent, not_null<Window::SessionController*> controller);

        [[nodiscard]] rpl::producer<QString> title() override;

    private:
        void SetupChats(not_null<Ui::VerticalLayout*> container);
        void SetupStickerShape(not_null<Ui::VerticalLayout*> container);
        void SetupQuickActions(not_null<Ui::VerticalLayout*> container,
                                        not_null<Window::SessionController*> controller);
        void SetupStickers(not_null<Ui::VerticalLayout*> container);

        void SetupRabbitChats(not_null<Ui::VerticalLayout*> container, not_null<Window::SessionController*> null);
        void setupContent(not_null<Window::SessionController*> controller);
    };
} // namespace Settings
