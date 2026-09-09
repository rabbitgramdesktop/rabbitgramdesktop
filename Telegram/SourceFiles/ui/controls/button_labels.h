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

namespace Ui {

class RoundButton;
class RpWidget;

void SetButtonTwoLabels(
    not_null<Ui::RoundButton*> button,
    rpl::producer<TextWithEntities> title,
    rpl::producer<TextWithEntities> subtitle,
    const style::FlatLabel &st,
    const style::FlatLabel &subst,
    const style::color *textFg = nullptr);

void SetButtonTwoLabels(
    not_null<Ui::RpWidget*> button,
    int singleLineTextTop,
    rpl::producer<TextWithEntities> title,
    rpl::producer<TextWithEntities> subtitle,
    const style::FlatLabel &st,
    const style::FlatLabel &subst,
    const style::color *textFg = nullptr);

} // namespace Ui