/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/rabbit_settings_menu.h"

#include <QtCore/QTemporaryFile>
#include <memory>

#include "api/api_common.h"
#include "core/application.h"
#include "core/file_utilities.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "main/main_session.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings_menu/sections/rabbit_appearance.h"
#include "rabbit/settings_menu/sections/rabbit_chats.h"
#include "rabbit/settings_menu/sections/rabbit_general.h"
#include "rabbit/settings_menu/sections/rabbit_other.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "storage/localimageloader.h"
#include "storage/localstorage.h"
#include "storage/storage_media_prepare.h"
#include "ui/basic_click_handlers.h"
#include "ui/chat/attach/attach_prepare.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_peer_menu.h"
#include "window/window_session_controller.h"
#include "apiwrap.h"
#include "mainwindow.h"

#include "styles/style_boxes.h"
#include "styles/style_menu_icons.h"

namespace Settings {
namespace {

using namespace Builder;

void BuildRabbitSettings(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/categories"_q,
		.title = rktr(u"rtg_settings_categories"_q),
		.keywords = { u"rabbit"_q, u"settings"_q, u"categories"_q },
	});

	builder.addSkip();
	builder.addSectionButton({
		.title = rktr(u"rtg_settings_general"_q),
		.targetSection = RabbitGeneral::Id(),
		.icon = { &st::menuIconShowAll },
		.keywords = { u"general"_q, u"basic"_q },
	});
	builder.addSectionButton({
		.title = rktr(u"rtg_settings_appearance"_q),
		.targetSection = RabbitAppearance::Id(),
		.icon = { &st::menuIconPalette },
		.keywords = { u"appearance"_q, u"theme"_q },
	});
	builder.addSectionButton({
		.title = rktr(u"rtg_settings_chats"_q),
		.targetSection = RabbitChats::Id(),
		.icon = { &st::menuIconChatBubble },
		.keywords = { u"chats"_q, u"messages"_q },
	});
	builder.addSectionButton({
		.title = rktr(u"rtg_settings_other"_q),
		.targetSection = RabbitOther::Id(),
		.icon = { &st::menuIconShowAll },
		.keywords = { u"other"_q, u"analytics"_q, u"telemetry"_q },
	});
}

void BuildRabbitLinks(SectionBuilder &builder) {
	builder.addSubsectionTitle({
		.id = u"rabbit/links"_q,
		.title = rktr(u"rtg_links"_q),
		.keywords = { u"links"_q, u"community"_q },
	});

	builder.addSkip();
	builder.addButton({
		.id = u"rabbit/links/channel"_q,
		.title = rktr(u"rtg_channel_title"_q),
		.icon = { &st::menuIconChannel },
		.label = rktr(u"rtg_channel_label"_q),
		.onClick = [] {
			Core::App().openLocalUrl(
				u"tg://resolve?domain=rabbitGramUpdates"_q,
				{});
		},
		.keywords = { u"channel"_q, u"updates"_q },
	});
	builder.addButton({
		.id = u"rabbit/links/group"_q,
		.title = rktr(u"rtg_group_title"_q),
		.icon = { &st::menuIconGroups },
		.label = rktr(u"rtg_group_label"_q),
		.onClick = [] {
			Core::App().openLocalUrl(
				u"tg://resolve?domain=rabbitGramDesktop"_q,
				{});
		},
		.keywords = { u"group"_q, u"community"_q },
	});
	builder.addButton({
		.id = u"rabbit/links/translate"_q,
		.title = rktr(u"rtg_translate_title"_q),
		.icon = { &st::menuIconTranslate },
		.label = rktr(u"rtg_translate_label"_q),
		.onClick = [] {
			UrlClickHandler::Open(
				u"https://crowdin.com/project/rabbitgramdesktop"_q);
		},
		.keywords = { u"translate"_q, u"localization"_q },
	});
	builder.addButton({
		.id = u"rabbit/links/source"_q,
		.title = rktr(u"rtg_source_title"_q),
		.icon = { &st::menuIconDelete },
		.label = rktr(u"rtg_source_label"_q),
		.onClick = [] {
			UrlClickHandler::Open(
				u"https://github.com/rabbitgramdesktop/rabbitgramdesktop"_q);
		},
		.keywords = { u"source"_q, u"github"_q },
	});
}

void BuildRabbitSection(SectionBuilder &builder) {
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

rpl::producer<QString> Rabbit::title() {
	return rktr(u"rtg_settings"_q);
}

void Rabbit::fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) {
	addAction(
		ktr(u"rtg_share_settings_to_chat"_q),
		[=] {
			const auto data = RabbitSettings::JsonSettings::ExportSettingsJson();
			auto tempFile = std::make_shared<QTemporaryFile>(
				QDir::tempPath() + u"/settings_XXXXXX.rtgconfig"_q);
			if (!tempFile->open()) {
				return;
			}
			tempFile->write(data);
			tempFile->flush();

			const auto tempPath = tempFile->fileName();
			const auto ctrl = controller();
			auto chosen = [ctrl, tempPath, tempFile](not_null<Data::Thread *> thread) mutable -> bool {
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
		ktr(u"rtg_share_settings"_q),
		[=] {
			const auto data = RabbitSettings::JsonSettings::ExportSettingsJson();
			FileDialog::GetWritePath(
				Core::App().getFileDialogParent(),
				u"Export rabbitGram Settings"_q,
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
		ktr(u"rtg_restart"_q),
		[] { Core::Restart(); },
		&st::menuIconRestore);
}

Rabbit::Rabbit(
	QWidget *parent,
	not_null<Window::SessionController *> controller)
	: Section(parent, controller) {
	setupContent();
}

void Rabbit::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kRabbitSection);
	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
