/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/flat_set.h"
#include "base/weak_qptr.h"
#include "ui/rp_widget.h"

namespace Ui {
class AbstractButton;
class ContinuousSlider;
} // namespace Ui

namespace Settings {

class KeyNavigation final {
public:
	explicit KeyNavigation(not_null<Ui::RpWidget*> inner);

	[[nodiscard]] bool handle(not_null<QKeyEvent*> e);
	void anchorTo(not_null<QWidget*> widget);

private:
	struct Entry {
		not_null<Ui::RpWidget*> widget;
		Ui::AbstractButton *button = nullptr;
		Ui::ContinuousSlider *slider = nullptr;
		QRect geometry;
		bool fullWidth = true;
		bool ownBackground = false;
	};

	[[nodiscard]] std::vector<Entry> list() const;
	void activate(
		not_null<Ui::AbstractButton*> button,
		Qt::KeyboardModifiers modifiers);
	void select(const std::vector<Entry> &entries, int index);
	void updateHighlight();
	void clearSelection();
	void reselectFromHidden();
	void track(not_null<Ui::RpWidget*> widget);

	const not_null<Ui::RpWidget*> _inner;
	base::weak_qptr<Ui::RpWidget> _selected;
	base::weak_qptr<QWidget> _anchor;
	base::flat_set<not_null<Ui::RpWidget*>> _tracked;
	Ui::RpWidget *_highlight = nullptr;
	QRect _selectedGeometry;
	bool _highlightRounded = false;
	rpl::lifetime _selectedLifetime;
	rpl::lifetime _lifetime;

};

} // namespace Settings
