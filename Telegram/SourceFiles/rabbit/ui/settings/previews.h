/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"

#include <memory>

namespace Ui {
class ChatStyle;
} // namespace Ui

class RoundnessPreview : public Ui::RpWidget
{
public:
    RoundnessPreview(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class ChatPreview : public Ui::RpWidget
{
public:
    ChatPreview(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    std::unique_ptr<Ui::ChatStyle> _chatStyle;
    int _paletteVersion = 0;

    void ensureChatStyle();
};

class StickerShapePicker : public Ui::RpWidget
{
public:
    StickerShapePicker(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
};
