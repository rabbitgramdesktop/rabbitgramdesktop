/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {
class GenericBox;
class VerticalLayout;
} // namespace Ui

class PeerData;

void UsernamesBox(
	not_null<Ui::GenericBox*> box,
	not_null<PeerData*> peer);

struct UsernameCheckInfo final {
	[[nodiscard]] static UsernameCheckInfo PurchaseAvailable(
		const QString &username,
		not_null<PeerData*> peer);

	enum class Type {
		Good,
		Error,
		Default,
	};
	Type type;
	TextWithEntities text;
};

void AddUsernameCheckLabel(
	not_null<Ui::VerticalLayout*> container,
	rpl::producer<UsernameCheckInfo> checkInfo);
