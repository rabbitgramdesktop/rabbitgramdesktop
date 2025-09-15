/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "editor/scene/scene_item_base.h"

namespace Editor {

class ItemImage : public ItemBase {
public:
	ItemImage(
		QPixmap &&pixmap,
		ItemBase::Data data);
	void paint(
		QPainter *p,
		const QStyleOptionGraphicsItem *option,
		QWidget *widget) override;
protected:
	void performFlip() override;
	std::shared_ptr<ItemBase> duplicate(ItemBase::Data data) const override;
private:
	QPixmap _pixmap;

};

} // namespace Editor
