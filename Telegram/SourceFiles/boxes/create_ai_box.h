/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/object_ptr.h"

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class GenericBox;
} // namespace Ui

namespace ChatHelpers {
class Show;
} // namespace ChatHelpers

namespace Iv {
struct RichPage;
} // namespace Iv

namespace Iv::Editor {

struct CreateAiBoxArgs {
	not_null<Main::Session*> session;
	Fn<void(std::shared_ptr<const RichPage>)> applyToPage;
};

void CreateAiBox(not_null<Ui::GenericBox*> box, CreateAiBoxArgs &&args);
void ShowCreateAiBox(
	std::shared_ptr<ChatHelpers::Show> show,
	CreateAiBoxArgs &&args);

} // namespace Iv::Editor
