/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace ChatHelpers {
class Show;
} // namespace ChatHelpers

namespace Data {
struct MessageReactionsTopPaid;
} // namespace Data

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class BoxContent;
class GenericBox;
} // namespace Ui

namespace Calls::Group::Ui {
using namespace ::Ui;
struct StarsColoring;
} // namespace Calls::Group::Ui

namespace Calls::Group {

[[nodiscard]] int MaxVideoStreamStarsCount(not_null<Main::Session*> session);

struct VideoStreamStarsBoxArgs {
	std::shared_ptr<ChatHelpers::Show> show;
	std::vector<Data::MessageReactionsTopPaid> top;
	int min = 0;
	int current = 0;
	bool sending = false;
	bool admin = false;
	Fn<void(int)> save;
	QString name;
};

void VideoStreamStarsBox(
	not_null<Ui::GenericBox*> box,
	VideoStreamStarsBoxArgs &&args);

[[nodiscard]] object_ptr<Ui::BoxContent> MakeVideoStreamStarsBox(
	VideoStreamStarsBoxArgs &&args);

} // namespace Calls::Group
