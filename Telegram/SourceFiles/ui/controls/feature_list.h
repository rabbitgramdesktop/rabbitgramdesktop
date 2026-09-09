/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace style {
struct FlatLabel;
} // namespace style

namespace st {
extern const style::FlatLabel &infoStarsFeatureTitle;
extern const style::FlatLabel &infoStarsFeatureAbout;
} // namespace st

namespace Ui::Text {
struct MarkedContext;
} // namespace Ui::Text

namespace Ui {

class RpWidget;

struct FeatureListEntry {
	const style::icon &icon;
	QString title;
	TextWithEntities about;
};

[[nodiscard]] object_ptr<RpWidget> MakeFeatureListEntry(
	QWidget *parent,
	FeatureListEntry feature,
	const Text::MarkedContext &context = {},
	const style::FlatLabel &stTitle = st::infoStarsFeatureTitle,
	const style::FlatLabel &stAbout = st::infoStarsFeatureAbout);

} // namespace Ui
