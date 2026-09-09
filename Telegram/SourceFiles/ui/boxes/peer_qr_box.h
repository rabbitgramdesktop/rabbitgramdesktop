/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class PeerData;

namespace Ui {

class GenericBox;
class Show;

void DefaultShowFillPeerQrBoxCallback(
	std::shared_ptr<Ui::Show> show,
	PeerData *peer);

void FillPeerQrBox(
	not_null<Ui::GenericBox*> box,
	PeerData *peer,
	std::optional<QString> customLink,
	rpl::producer<QString> about);

} // namespace Ui
