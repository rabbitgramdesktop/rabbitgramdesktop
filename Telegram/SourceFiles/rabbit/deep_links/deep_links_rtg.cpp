/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/deep_links/deep_links_rtg.h"

#include "core/deep_links/deep_links_router.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/settings_menu/sections/rabbit_general.h"
#include "rabbit/settings_menu/sections/rabbit_appearance.h"
#include "rabbit/settings_menu/sections/rabbit_chats.h"
#include "window/window_session_controller.h"

namespace Core::DeepLinks {
namespace {

struct RtgSectionMapping {
	QString pathPrefix;
	Settings::Type sectionId;
};

[[nodiscard]] const std::vector<RtgSectionMapping> &SectionMappings() {
	static const auto result = std::vector<RtgSectionMapping>{
		{ u"general"_q, Settings::RabbitGeneral::Id() },
		{ u"appearance"_q, Settings::RabbitAppearance::Id() },
		{ u"chats"_q, Settings::RabbitChats::Id() },
	};
	return result;
}

Result HandleRtgSettingsWithSuffix(
		const Context &ctx,
		const QString &pathPrefix,
		Settings::Type sectionId) {
	if (!ctx.controller) {
		return Result::NeedsAuth;
	}
	const auto fullPath = ctx.path;
	const auto expectedPrefix = u"settings/"_q + pathPrefix + '/';
	if (fullPath.startsWith(expectedPrefix)) {
		const auto controlSuffix = fullPath.mid(expectedPrefix.size());
		if (!controlSuffix.isEmpty()) {
			const auto controlId = u"rabbit/"_q
				+ pathPrefix
				+ '/'
				+ controlSuffix;
			ctx.controller->setHighlightControlId(controlId);
		}
	}
	ctx.controller->showSettings(sectionId);
	return Result::Handled;
}

} // namespace

QString RtgSettingsDeepLink(const QString &controlId) {
	if (!controlId.startsWith(u"rabbit/"_q)) {
		return QString();
	}
	const auto suffix = controlId.mid(7);
	return u"tg://rtg/settings/"_q + suffix;
}

void RegisterRtgHandlers(Router &router) {
	router.add(u"rtg"_q, {
		.path = QString(),
		.action = SettingsSection{ Settings::Rabbit::Id() },
	});

	router.add(u"rtg"_q, {
		.path = u"settings"_q,
		.action = SettingsSection{ Settings::Rabbit::Id() },
	});

	for (const auto &mapping : SectionMappings()) {
		const auto prefix = mapping.pathPrefix;
		const auto sectionId = mapping.sectionId;

		router.add(u"rtg"_q, {
			.path = u"settings/"_q + prefix,
			.action = CodeBlock{ [=](const Context &ctx) {
				return HandleRtgSettingsWithSuffix(ctx, prefix, sectionId);
			}},
		});
	}
}

} // namespace Core::DeepLinks
