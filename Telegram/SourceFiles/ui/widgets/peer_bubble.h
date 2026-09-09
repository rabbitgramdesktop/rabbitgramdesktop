/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

template <typename Object>
class object_ptr;

class PeerData;

namespace Ui {
class RpWidget;
class VerticalLayout;
} // namespace Ui

namespace Ui {

[[nodiscard]] object_ptr<Ui::RpWidget> CreatePeerBubble(
	not_null<Ui::RpWidget*> parent,
	not_null<PeerData*> peer);

} // namespace Ui
