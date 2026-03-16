/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include <ui/boxes/single_choice_box.h>

#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"

#include "rabbit/settings_menu/sections/rabbit_appearance.h"
#include "rabbit/settings_menu/sections/rabbit_chats.h"
#include "rabbit/settings_menu/sections/rabbit_general.h"

#include "core/application.h"
#include "core/file_utilities.h"
#include "lang_auto.h"
#include "mainwindow.h"
#include "settings/settings_common.h"
#include "settings/settings_builder.h"
#include "settings/sections/settings_main.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/checkbox.h"
#include "ui/vertical_list.h"
#include "boxes/connection_box.h"
#include "platform/platform_specific.h"
#include "window/window_session_controller.h"
#include "lang/lang_instance.h"
#include "core/application.h"
#include "storage/localstorage.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "ui/toast/toast.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "apiwrap.h"
#include "api/api_blocked_peers.h"
#include "api/api_common.h"
#include "data/data_user.h"
#include "storage/localimageloader.h"
#include "storage/storage_media_prepare.h"
#include "ui/chat/attach/attach_prepare.h"
#include "window/window_peer_menu.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/basic_click_handlers.h"
#include "styles/style_chat.h"
#include "styles/style_boxes.h"

namespace Settings
{
    namespace
    {
        using namespace Builder;

        void BuildRabbitSettings(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/categories"_q,
                .title = rktr("rtg_settings_categories"),
                .keywords = { u"rabbit"_q, u"settings"_q, u"categories"_q },
            });

            builder.addSkip();
            builder.addSectionButton({
                .title = rktr("rtg_settings_general"),
                .targetSection = RabbitGeneral::Id(),
                .icon = { &st::menuIconShowAll },
                .keywords = { u"general"_q, u"basic"_q },
            });
            builder.addSectionButton({
                .title = rktr("rtg_settings_appearance"),
                .targetSection = RabbitAppearance::Id(),
                .icon = { &st::menuIconPalette },
                .keywords = { u"appearance"_q, u"theme"_q },
            });
            builder.addSectionButton({
                .title = rktr("rtg_settings_chats"),
                .targetSection = RabbitChats::Id(),
                .icon = { &st::menuIconChatBubble },
                .keywords = { u"chats"_q, u"messages"_q },
            });
        }

        void BuildRabbitLinks(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/links"_q,
                .title = rktr("rtg_links"),
                .keywords = { u"links"_q, u"community"_q },
            });

            builder.addSkip();
            builder.addButton({
                .id = u"rabbit/links/channel"_q,
                .title = rktr("rtg_channel_title"),
                .icon = { &st::menuIconChannel },
                .label = rktr("rtg_channel_label"),
                .onClick = [] {
                    Core::App().openLocalUrl(
                        "tg://resolve?domain=rabbitGramUpdates",
                        {});
                },
                .keywords = { u"channel"_q, u"updates"_q },
            });
            builder.addButton({
                .id = u"rabbit/links/group"_q,
                .title = rktr("rtg_group_title"),
                .icon = { &st::menuIconGroups },
                .label = rktr("rtg_group_label"),
                .onClick = [] {
                    Core::App().openLocalUrl(
                        "tg://resolve?domain=rabbitGramDesktop",
                        {});
                },
                .keywords = { u"group"_q, u"community"_q },
            });
            builder.addButton({
                .id = u"rabbit/links/translate"_q,
                .title = rktr("rtg_translate_title"),
                .icon = { &st::menuIconTranslate },
                .label = rktr("rtg_translate_label"),
                .onClick = [] {
                    UrlClickHandler::Open(
                        "https://crowdin.com/project/rabbitgramdesktop");
                },
                .keywords = { u"translate"_q, u"localization"_q },
            });
            builder.addButton({
                .id = u"rabbit/links/source"_q,
                .title = rktr("rtg_source_title"),
                .icon = { &st::menuIconDelete },
                .label = rktr("rtg_source_label"),
                .onClick = [] {
                    UrlClickHandler::Open(
                        "https://github.com/rabbitgramdesktop/rabbitgramdesktop");
                },
                .keywords = { u"source"_q, u"github"_q },
            });
        }

        void BuildRabbitSection(SectionBuilder &builder)
        {
            builder.addSkip();
            BuildRabbitSettings(builder);
            builder.addSkip();
            builder.addDivider();
            builder.addSkip();
            BuildRabbitLinks(builder);
        }

        const auto kMeta = BuildHelper({
            .id = Rabbit::Id(),
            .parentId = MainId(),
            .title = rktr_phrase(u"rtg_settings"_q),
            .icon = &st::menuIconRabbit,
        }, [](SectionBuilder &builder) {
            BuildRabbitSection(builder);
        });

        const SectionBuildMethod kRabbitSection = kMeta.build;
    } // namespace

    rpl::producer<QString> Rabbit::title()
    {
        return rktr("rtg_settings");
    }

    void Rabbit::fillTopBarMenu(
            const Ui::Menu::MenuCallback &addAction) {
        addAction(
            ktr("rtg_share_settings_to_chat"),
            [=] {
                const auto data = RabbitSettings::JsonSettings::ExportSettingsJson();
                const auto tempPath = QDir::temp().filePath(u"settings.rtgconfig"_q);
                auto f = QFile(tempPath);
                if (!f.open(QIODevice::WriteOnly)) {
                    return;
                }
                f.write(data);
                f.close();

                const auto ctrl = controller();
                auto chosen = [=](not_null<Data::Thread*> thread) mutable -> bool {
                    auto list = Storage::PrepareMediaList(
                        QStringList{ tempPath },
                        st::sendMediaPreviewSize,
                        ctrl->session().user()->isPremium());
                    if (list.error != Ui::PreparedList::Error::None) {
                        return false;
                    }
                    auto action = Api::SendAction(thread);
                    action.clearDraft = false;
                    ctrl->session().api().sendFiles(
                        std::move(list),
                        SendMediaType::File,
                        nullptr,
                        action);
                    return true;
                };
                Window::ShowChooseRecipientBox(ctrl, std::move(chosen));
            },
            &st::menuIconShare);
        addAction(
            ktr("rtg_share_settings"),
            [=] {
                const auto data = RabbitSettings::JsonSettings::ExportSettingsJson();
                FileDialog::GetWritePath(
                    Core::App().getFileDialogParent(),
                    u"Export RabbitGram Settings"_q,
                    u"RTG Config (*.rtgconfig)"_q,
                    u"settings.rtgconfig"_q,
                    [=](QString &&path) {
                        if (path.isEmpty()) {
                            return;
                        }
                        auto f = QFile(path);
                        if (!f.open(QIODevice::WriteOnly)) {
                            return;
                        }
                        f.write(data);
                    });
            },
            &st::menuIconExport);
        addAction(
            ktr("rtg_restart"),
            [] { Core::Restart(); },
            &st::menuIconRestore);
    }

    Rabbit::Rabbit(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent();
    }

    void Rabbit::setupContent()
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
        build(content, kRabbitSection);
        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
