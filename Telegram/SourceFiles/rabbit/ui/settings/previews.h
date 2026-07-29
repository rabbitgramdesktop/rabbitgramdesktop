/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"
#include "history/view/history_view_element.h"
#include "history/admin_log/history_admin_log_item.h"

#include <memory>

namespace Ui {
class ChatStyle;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

class History;

class RoundnessPreview final : public Ui::RpWidget {
public:
	RoundnessPreview(QWidget *parent);

protected:
	void paintEvent(QPaintEvent *e) override;

};

class ChatPreview final
	: public Ui::RpWidget
	, private HistoryView::SimpleElementDelegate {
public:
	ChatPreview(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

protected:
	void paintEvent(QPaintEvent *e) override;

private:
	not_null<HistoryView::ElementDelegate*> delegate();
	HistoryView::Context elementContext() override;

	const not_null<Window::SessionController*> _controller;
	std::unique_ptr<Ui::ChatStyle> _chatStyle;
	int _paletteVersion = 0;
	int _messageRoundness = -1;

	const not_null<History*> _history;
	AdminLog::OwnedItem _item1;
	AdminLog::OwnedItem _item2;
	AdminLog::OwnedItem _item3;
	AdminLog::OwnedItem _item4;

};

class StickerShapePicker final : public Ui::RpWidget {
public:
	StickerShapePicker(QWidget *parent);

protected:
	void paintEvent(QPaintEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;

};

class QuickActionsPreview final : public Ui::RpWidget {
public:
	QuickActionsPreview(QWidget *parent);

protected:
	void paintEvent(QPaintEvent *e) override;

};
