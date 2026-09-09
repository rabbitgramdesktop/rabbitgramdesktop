/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Api {

class RemoveComplexChatFilter final {
public:
	RemoveComplexChatFilter();

	void request(
		base::weak_qptr<Ui::RpWidget> widget,
		base::weak_ptr<Window::SessionController> weak,
		FilterId id);

private:
	FilterId _removingId = 0;
	mtpRequestId _removingRequestId = 0;

};

} // namespace Api
