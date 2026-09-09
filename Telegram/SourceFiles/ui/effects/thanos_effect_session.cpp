/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/effects/thanos_effect_session.h"

#include "ui/effects/thanos_effect.h"
#include "data/data_folder.h"
#include "data/data_session.h"
#include "main/main_session.h"

namespace Ui {

void ScheduleThanosEffectWarmUp(
		not_null<Main::Session*> session,
		rpl::lifetime &lifetime) {
	if (session->data().chatsListLoaded(nullptr)) {
		ThanosEffect::WarmUp();
		return;
	}
	session->data().chatsListLoadedEvents(
	) | rpl::filter([](Data::Folder *folder) {
		return !folder;
	}) | rpl::take(1) | rpl::on_next([] {
		ThanosEffect::WarmUp();
	}, lifetime);
}

} // namespace Ui
