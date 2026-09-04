/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QString>
#include <optional>

namespace Rabbit {

struct MathEvaluationResult {
	QString expression;
	double value = 0.0;
	QString formattedResult;
};

class MathEvaluator final {
public:
	[[nodiscard]] static std::optional<MathEvaluationResult> EvaluateExpressionEnd(
		const QString &text);
	[[nodiscard]] static std::optional<double> Evaluate(
		const QString &expression);
};

} // namespace Rabbit
