/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "previews.h"

#include "rabbit/settings/rabbit_settings.h"

#include "core/application.h"
#include "styles/style_layers.h"

#include "main/main_domain.h"
#include "styles/style_rabbit_assets.h"
#include "styles/style_chat.h"
#include "styles/style_widgets.h"
#include "ui/painter.h"
#include "ui/chat/chat_style_radius.h"
#include "window/main_window.h"

RoundnessPreview::RoundnessPreview(QWidget* parent) : RpWidget(parent) {
    auto sectionHeight = st::rndPreviewSize;
    setMinimumSize(st::boxWidth, sectionHeight);
}

void RoundnessPreview::paintEvent(QPaintEvent* e) {
    Painter p(this);
    PainterHighQualityEnabler hq(p);

    auto size = st::rndPreviewSize;
    auto radius = size * (RabbitSettings::userpicRoundness() / 100.);

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(st::rndPreviewFill));
    p.drawRoundedRect(
        0, 0,
        size, size,
        radius, radius
    );

    p.setBrush(QBrush(st::rndSkeletonFill));
    auto skeletonWidth = st::boxWidth - (3 * st::rndPreviewSize);
    auto skeletonHeight = st::rndPreviewSize / 5;
    p.drawRoundedRect(
        st::rndPreviewSize * 1.33,
        skeletonHeight,
        skeletonWidth / 2,
        skeletonHeight,
        skeletonHeight / 2,
        skeletonHeight / 2
    );

    p.drawRoundedRect(
        st::rndPreviewSize * 1.33,
        skeletonHeight * 3,
        skeletonWidth,
        skeletonHeight,
        skeletonHeight / 2,
        skeletonHeight / 2
    );
}

ChatPreview::ChatPreview(QWidget* parent) : RpWidget(parent) {
    auto sectionHeight = st::stickerPreviewSize;
    setMinimumSize(st::boxWidth, sectionHeight);
}

void ChatPreview::paintEvent(QPaintEvent* e) {
    Painter p(this);
    PainterHighQualityEnabler hq(p);

    auto sticker_size = RabbitSettings::stickerSize();
    auto size = QSize(sticker_size, sticker_size * 0.7);
    auto radius = []() -> qreal {
        switch (RabbitSettings::stickerShape()) {
        case 1: return st::bubbleRadiusSmall;
        case 2: return st::bubbleRadiusLarge;
        default: return 0;
        }
    };
    auto message_radius = []() -> int {
        return Ui::BubbleRadiusLarge();
    };

    p.setPen(Qt::NoPen);
    p.setBrush(st::rndPreviewFill);
    p.drawRoundedRect(
        QRect(QPoint(0, 0), size),
        radius(), radius());

    p.setBrush(QBrush(st::rndSkeletonFill));
    p.drawRoundedRect(
        size.width() + st::stickerPreviewMargin,
        size.height() - st::stickerPreviewTimeHeight,
        st::stickerPreviewTimeWidth,
        st::stickerPreviewTimeHeight,
        st::stickerPreviewTimeHeight / 2.,
        st::stickerPreviewTimeHeight / 2.
    );

    auto multipliers = { 1.7, 1.5, 1., 1.2 };
    auto topPadding = size.height() + st::stickerPreviewMargin;

    for (auto multiplier : multipliers) {
        auto spacefillerMsgSkeletonWidth = (st::boxWidth / 2) * multiplier;

        p.drawRoundedRect(
            st::boxWidth - spacefillerMsgSkeletonWidth, 
            topPadding,
            spacefillerMsgSkeletonWidth,
            st::stickerSpacefillerHeight,
            message_radius(),
            message_radius()
        );

        topPadding += st::stickerPreviewMargin + st::stickerSpacefillerHeight;
    }
}

StickerShapePicker::StickerShapePicker(QWidget* parent) : RpWidget(parent) {
    setMinimumSize(st::stickerShapeBoxWidth, st::stickerShapeBoxHeight);
}

void StickerShapePicker::paintEvent(QPaintEvent* e) {
    Painter p(this);
    PainterHighQualityEnabler hq(p);
    
    auto activePen = QPen(st::windowBgActive, st::stickerShapePenWidth);
    auto inactivePen = QPen(st::rndSkeletonFill, st::stickerShapePenWidth);
    auto stickerBrush = QBrush(st::rndPreviewFill);

    auto variantCardWidth = st::stickerShapeVariantCardWidth;
    auto variantCardHeight = st::stickerShapeVariantCardHeight;
    auto variantCardMargin = st::stickerShapeMargins;

    auto variantPadding = st::stickerShapePadding;
    auto variantWidth = variantCardWidth - 2 * variantPadding - 2 * st::stickerShapePenWidth;
    auto variantHeight = variantCardHeight - 2 * variantPadding /* - 2 * st::stickerShapePenWidth */;
    
    auto radiuses = [](int index) -> int {
        switch (index) {
            case 1: return st::bubbleRadiusSmall;
            case 2: return st::bubbleRadiusLarge;
            default: return 0;
        }
    };
    
    auto gapLeft = st::stickerShapePenWidth;

    for (int i = 0; i < 3; i++) {
        p.setPen(RabbitSettings::stickerShape() == i
            ? activePen
            : inactivePen);
        p.setBrush(Qt::NoBrush);

        p.drawRoundedRect(
            gapLeft, st::stickerShapePenWidth,
            variantCardWidth, variantCardHeight,
            st::bubbleRadiusSmall, st::bubbleRadiusSmall);

        p.setPen(Qt::NoPen);
        p.setBrush(st::rndPreviewFill);

        auto rect = QRect(
            gapLeft + variantPadding + st::stickerShapePenWidth,
            variantPadding + st::stickerShapePenWidth,
            variantWidth, variantHeight);

        p.drawRoundedRect(rect,
            radiuses(i), radiuses(i));
        
        gapLeft += variantCardWidth + variantCardMargin;
    }
}

void StickerShapePicker::mousePressEvent(QMouseEvent *e) {
    auto variantWidth = st::stickerShapeVariantCardWidth;
    auto variantMargin = st::stickerShapeMargins;

    auto x = e->pos().x();

    for (int i = 0; i < 3; i++) {
        auto maxCords = (i + 1) * variantWidth + (i + 1) * variantMargin;
        if (x < maxCords) {
            RabbitSettings::setStickerShape(i);
            repaint();
            break;
        }
    }
}