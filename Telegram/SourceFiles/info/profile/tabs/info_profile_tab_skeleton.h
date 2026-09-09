/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/object_ptr.h"
#include "storage/storage_shared_media.h"

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Info::Profile {

[[nodiscard]] object_ptr<Ui::RpWidget> CreateTabSkeleton(
	not_null<QWidget*> parent,
	Storage::SharedMediaType type);

} // namespace Info::Profile
