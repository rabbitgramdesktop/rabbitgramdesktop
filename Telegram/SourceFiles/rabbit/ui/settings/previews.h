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

namespace Data {
class DocumentMedia;
} // namespace Data

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

	int resizeGetHeight(int newWidth) override;
	void updateItems();

protected:
	void paintEvent(QPaintEvent *e) override;

private:
	not_null<HistoryView::ElementDelegate*> delegate();
	HistoryView::Context elementContext() override;
	void regenerateItems();

	const not_null<Window::SessionController*> _controller;
	std::unique_ptr<Ui::ChatStyle> _chatStyle;
	int _paletteVersion = 0;
	int _messageRoundness = -1;

	const not_null<History*> _history;
	std::shared_ptr<Data::DocumentMedia> _stickerMedia;
	AdminLog::OwnedItem _item0;
	AdminLog::OwnedItem _item1;
	AdminLog::OwnedItem _item2;

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

class FoldersPreview final : public Ui::RpWidget {
public:
	FoldersPreview(
		QWidget *parent,
		Window::SessionController *controller = nullptr);

	int resizeGetHeight(int newWidth) override;

protected:
	void paintEvent(QPaintEvent *e) override;

private:
	Window::SessionController *_controller = nullptr;
	int _activeTab = 0;

};
