/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class HistoryItem;
struct HistoryMessageMarkupButton;

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class GenericBox;
class Show;
} // namespace Ui

namespace UrlAuthBox {

void ActivateButton(
	std::shared_ptr<Ui::Show> show,
	not_null<const HistoryItem*> message,
	int row,
	int column);
void ActivateUrl(
	std::shared_ptr<Ui::Show> show,
	not_null<Main::Session*> session,
	const QString &url,
	QVariant context);

} // namespace UrlAuthBox
