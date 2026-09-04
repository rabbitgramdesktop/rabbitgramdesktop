/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/math_evaluator/math_autocomplete.h"
#include "rabbit/math_evaluator/math_evaluator.h"
#include "rabbit/settings/rabbit_settings.h"

#include "ui/rp_widget.h"
#include "styles/style_basic.h"
#include "styles/style_widgets.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QTextEdit>
#include <rpl/rpl.h>

namespace Rabbit {
namespace {

class MathGhostOverlay final : public Ui::RpWidget {
public:
	explicit MathGhostOverlay(not_null<QWidget*> parent)
	: RpWidget(parent) {
		setAttribute(Qt::WA_TransparentForMouseEvents);
	}

	void setSuggestion(const QString &text, const QPoint &pos, const QFont &font) {
		_text = text;
		_pos = pos;
		_font = font;
		update();
	}

	void clearSuggestion() {
		if (!_text.isEmpty()) {
			_text.clear();
			update();
		}
	}

	[[nodiscard]] bool hasSuggestion() const {
		return !_text.isEmpty();
	}

protected:
	void paintEvent(QPaintEvent *e) override {
		if (_text.isEmpty()) {
			return;
		}
		QPainter p(this);
		p.setFont(_font);
		p.setPen(QColor(140, 140, 140, 160));
		p.drawText(_pos, _text);
	}

private:
	QString _text;
	QPoint _pos;
	QFont _font;
};

class MathAutocompleteController final : public QObject {
public:
	explicit MathAutocompleteController(not_null<Ui::InputField*> field)
	: QObject(field)
	, _field(field) {
		const auto rawEdit = _field->rawTextEdit();
		_overlay = std::make_unique<MathGhostOverlay>(rawEdit);
		_overlay->setGeometry(rawEdit->rect());

		rawEdit->installEventFilter(this);

		rpl::merge(
			_field->changes(),
			_field->sizeValue() | rpl::to_empty,
			_field->scrollTop().changes() | rpl::to_empty
		) | rpl::on_next([=] {
			updateSuggestion();
		}, _lifetime);

		RabbitSettings::JsonSettings::Events(u"math_autocomplete"_q) | rpl::on_next([=] {
			updateSuggestion();
		}, _lifetime);
	}

protected:
	bool eventFilter(QObject *watched, QEvent *event) override {
		if (watched == _field->rawTextEdit() && event->type() == QEvent::KeyPress) {
			auto *k = static_cast<QKeyEvent*>(event);
			if (_overlay && _overlay->hasSuggestion()) {
				if (k->key() == Qt::Key_Space || k->key() == Qt::Key_Tab) {
					applySuggestion(k->key() == Qt::Key_Space);
					return true;
				} else if (k->key() == Qt::Key_Escape) {
					_suggestionText.clear();
					_overlay->clearSuggestion();
					return true;
				}
			}
		}
		return QObject::eventFilter(watched, event);
	}

private:
	void updateSuggestion() {
		if (!RabbitSettings::mathAutocomplete()) {
			clearSuggestion();
			return;
		}

		const auto rawEdit = _field->rawTextEdit();
		_overlay->setGeometry(rawEdit->rect());

		const auto text = _field->getTextWithTags().text;
		const auto cursor = _field->textCursor();
		const int cursorPos = cursor.position();

		if (cursorPos != text.length()) {
			clearSuggestion();
			return;
		}

		const auto evalResult = MathEvaluator::EvaluateExpressionEnd(text);
		if (!evalResult) {
			clearSuggestion();
			return;
		}

		_suggestionText = evalResult->formattedResult;

		const QRect cursorRect = rawEdit->cursorRect(cursor);
		const auto &stFont = _field->st().style.font;
		const int textX = cursorRect.x() + (text.endsWith('=') ? stFont->width(u" "_q) : 0);
		const int textY = cursorRect.y() + stFont->ascent;

		_overlay->setSuggestion(_suggestionText, QPoint(textX, textY), stFont->f);
	}

	void applySuggestion(bool withSpace) {
		if (_suggestionText.isEmpty()) {
			return;
		}
		const auto insert = _suggestionText + (withSpace ? u" "_q : u""_q);
		_suggestionText.clear();
		_overlay->clearSuggestion();

		auto cursor = _field->textCursor();
		cursor.insertText(insert);
		_field->setTextCursor(cursor);
	}

	void clearSuggestion() {
		_suggestionText.clear();
		if (_overlay) {
			_overlay->clearSuggestion();
		}
	}

	not_null<Ui::InputField*> _field;
	std::unique_ptr<MathGhostOverlay> _overlay;
	QString _suggestionText;
	rpl::lifetime _lifetime;
};

} // namespace

void AttachMathAutocomplete(not_null<Ui::InputField*> field) {
	new MathAutocompleteController(field);
}

} // namespace Rabbit
