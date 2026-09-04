/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/math_evaluator/math_evaluator.h"

#include <cmath>
#include <cctype>
#include <QtCore/QChar>

namespace Rabbit {
namespace {

constexpr auto kPi = 3.14159265358979323846;
constexpr auto kE = 2.71828182845904523536;

enum class TokenType {
	Number,
	Operator,
	Identifier,
	OpenParen,
	CloseParen,
	End,
	Error,
};

struct Token {
	TokenType type = TokenType::Error;
	double numberValue = 0.0;
	QString textValue;
};

class Tokenizer {
public:
	explicit Tokenizer(const QString &input)
	: _input(input)
	, _len(input.length()) {
	}

	Token next() {
		skipWhitespace();
		if (_pos >= _len) {
			return { TokenType::End, 0.0, QString() };
		}

		const auto ch = _input[_pos];

		if (ch.isDigit() || ch == '.' || ch == ',') {
			return readNumber();
		}

		if (ch.isLetter()) {
			return readIdentifier();
		}

		if (ch == '(') {
			_pos++;
			return { TokenType::OpenParen, 0.0, u"("_q };
		}
		if (ch == ')') {
			_pos++;
			return { TokenType::CloseParen, 0.0, u")"_q };
		}

		if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^') {
			_pos++;
			return { TokenType::Operator, 0.0, QString(ch) };
		}

		_pos++;
		return { TokenType::Error, 0.0, QString(ch) };
	}

private:
	void skipWhitespace() {
		while (_pos < _len && _input[_pos].isSpace()) {
			_pos++;
		}
	}

	Token readNumber() {
		const auto start = _pos;
		bool hasDot = false;
		while (_pos < _len) {
			const auto c = _input[_pos];
			if (c.isDigit()) {
				_pos++;
			} else if ((c == '.' || c == ',') && !hasDot) {
				hasDot = true;
				_pos++;
			} else {
				break;
			}
		}
		auto numStr = _input.mid(start, _pos - start);
		numStr.replace(',', '.');
		bool ok = false;
		const auto val = numStr.toDouble(&ok);
		if (!ok) {
			return { TokenType::Error, 0.0, numStr };
		}
		return { TokenType::Number, val, numStr };
	}

	Token readIdentifier() {
		const auto start = _pos;
		while (_pos < _len && (_input[_pos].isLetter() || _input[_pos].isDigit())) {
			_pos++;
		}
		const auto name = _input.mid(start, _pos - start).toLower();
		return { TokenType::Identifier, 0.0, name };
	}

	QString _input;
	int _pos = 0;
	int _len = 0;
};

class Parser {
public:
	explicit Parser(const QString &input)
	: _tokenizer(input) {
		consume();
	}

	std::optional<double> parse() {
		if (_currentToken.type == TokenType::End) {
			return std::nullopt;
		}
		const auto val = parseExpression();
		if (!val || _currentToken.type != TokenType::End) {
			return std::nullopt;
		}
		if (std::isnan(*val) || std::isinf(*val)) {
			return std::nullopt;
		}
		return val;
	}

private:
	void consume() {
		_currentToken = _tokenizer.next();
	}

	std::optional<double> parseExpression() {
		auto left = parseTerm();
		if (!left) {
			return std::nullopt;
		}

		while (_currentToken.type == TokenType::Operator
			&& (_currentToken.textValue == u"+"_q || _currentToken.textValue == u"-"_q)) {
			const auto op = _currentToken.textValue;
			consume();

			auto right = parseTerm();
			if (!right) {
				return std::nullopt;
			}

			if (op == u"+"_q) {
				*left += *right;
			} else {
				*left -= *right;
			}
		}
		return left;
	}

	std::optional<double> parseTerm() {
		auto left = parsePower();
		if (!left) {
			return std::nullopt;
		}

		while (_currentToken.type == TokenType::Operator
			&& (_currentToken.textValue == u"*"_q
				|| _currentToken.textValue == u"/"_q
				|| _currentToken.textValue == u"%"_q)) {
			const auto op = _currentToken.textValue;
			consume();
			auto right = parsePower();
			if (!right) {
				return std::nullopt;
			}
			if (op == u"*"_q) {
				*left *= *right;
			} else if (op == u"/"_q) {
				if (std::abs(*right) < 1e-12) {
					return std::nullopt;
				}
				*left /= *right;
			} else if (op == u"%"_q) {
				if (std::abs(*right) < 1e-12) {
					return std::nullopt;
				}
				*left = std::fmod(*left, *right);
			}
		}
		return left;
	}

	std::optional<double> parsePower() {
		auto left = parseUnary();
		if (!left) {
			return std::nullopt;
		}
		if (_currentToken.type == TokenType::Operator && _currentToken.textValue == u"^"_q) {
			consume();
			auto right = parsePower();
			if (!right) {
				return std::nullopt;
			}
			*left = std::pow(*left, *right);
		}
		return left;
	}

	std::optional<double> parseUnary() {
		if (_currentToken.type == TokenType::Operator && _currentToken.textValue == u"-"_q) {
			consume();
			auto val = parseUnary();
			return val ? std::make_optional(-(*val)) : std::nullopt;
		}
		if (_currentToken.type == TokenType::Operator && _currentToken.textValue == u"+"_q) {
			consume();
			return parseUnary();
		}
		return parsePostfix();
	}

	std::optional<double> parsePostfix() {
		auto val = parseFactor();
		if (!val) {
			return std::nullopt;
		}
		if (_currentToken.type == TokenType::Operator && _currentToken.textValue == u"%"_q) {
			consume();
			*val /= 100.0;
		}
		return val;
	}

	std::optional<double> parseFactor() {
		if (_currentToken.type == TokenType::Number) {
			const auto v = _currentToken.numberValue;
			consume();
			return v;
		}

		if (_currentToken.type == TokenType::OpenParen) {
			consume();
			auto v = parseExpression();
			if (!v || _currentToken.type != TokenType::CloseParen) {
				return std::nullopt;
			}
			consume();
			return v;
		}

		if (_currentToken.type == TokenType::Identifier) {
			const auto name = _currentToken.textValue;
			consume();

			if (name == u"pi"_q) {
				return kPi;
			}
			if (name == u"e"_q) {
				return kE;
			}

			if (_currentToken.type != TokenType::OpenParen) {
				return std::nullopt;
			}
			consume();
			auto arg = parseExpression();
			if (!arg || _currentToken.type != TokenType::CloseParen) {
				return std::nullopt;
			}
			consume();

			if (name == u"sin"_q) return std::sin(*arg);
			if (name == u"cos"_q) return std::cos(*arg);
			if (name == u"tan"_q) return std::tan(*arg);
			if (name == u"asin"_q) return std::asin(*arg);
			if (name == u"acos"_q) return std::acos(*arg);
			if (name == u"atan"_q) return std::atan(*arg);
			if (name == u"sqrt"_q) return (*arg >= 0) ? std::make_optional(std::sqrt(*arg)) : std::nullopt;
			if (name == u"cbrt"_q) return std::cbrt(*arg);
			if (name == u"abs"_q) return std::abs(*arg);
			if (name == u"log"_q || name == u"log10"_q) return (*arg > 0) ? std::make_optional(std::log10(*arg)) : std::nullopt;
			if (name == u"ln"_q) return (*arg > 0) ? std::make_optional(std::log(*arg)) : std::nullopt;
			if (name == u"exp"_q) return std::exp(*arg);
			if (name == u"floor"_q) return std::floor(*arg);
			if (name == u"ceil"_q) return std::ceil(*arg);
			if (name == u"round"_q) return std::round(*arg);

			return std::nullopt;
		}

		return std::nullopt;
	}

	Tokenizer _tokenizer;
	Token _currentToken;
};

} // namespace

std::optional<double> MathEvaluator::Evaluate(const QString &expression) {
	Parser parser(expression);
	return parser.parse();
}

std::optional<MathEvaluationResult> MathEvaluator::EvaluateExpressionEnd(const QString &text) {
	if (text.isEmpty()) {
		return std::nullopt;
	}

	int equalsPos = -1;
	for (int i = text.length() - 1; i >= 0; --i) {
		if (text[i] == '=') {
			equalsPos = i;
			break;
		} else if (!text[i].isSpace()) {
			return std::nullopt;
		}
	}

	if (equalsPos < 0) {
		return std::nullopt;
	}

	int startPos = equalsPos - 1;
	while (startPos >= 0 && text[startPos].isSpace()) {
		startPos--;
	}

	if (startPos < 0) {
		return std::nullopt;
	}

	int exprStart = startPos;
	int parenDepth = 0;
	bool hasOperatorOrFunction = false;

	while (exprStart >= 0) {
		const auto ch = text[exprStart];

		if (ch == ')') {
			parenDepth++;
			exprStart--;
			continue;
		}
		if (ch == '(') {
			parenDepth--;
			if (parenDepth < 0) {
				exprStart++;
				break;
			}
			exprStart--;
			continue;
		}

		if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^') {
			hasOperatorOrFunction = true;
			exprStart--;
			continue;
		}

		if (ch.isDigit() || ch == '.' || ch == ',' || ch.isSpace()) {
			exprStart--;
			continue;
		}

		if (ch.isLetter()) {
			hasOperatorOrFunction = true;
			exprStart--;
			continue;
		}

		exprStart++;
		break;
	}

	if (exprStart < 0) {
		exprStart = 0;
	}

	const auto candidate = text.mid(exprStart, startPos - exprStart + 1).trimmed();
	if (candidate.isEmpty() || !hasOperatorOrFunction) {
		return std::nullopt;
	}

	const auto evaluated = Evaluate(candidate);
	if (!evaluated) {
		return std::nullopt;
	}

	MathEvaluationResult result;
	result.expression = candidate;
	result.value = *evaluated;

	const double absVal = std::abs(result.value);
	if (std::abs(result.value - std::round(result.value)) < 1e-9 && absVal < 1e15) {
		result.formattedResult = QString::number(qRound64(result.value));
	} else {
		result.formattedResult = QString::number(result.value, 'f', 6);
		while (result.formattedResult.endsWith('0')) {
			result.formattedResult.chop(1);
		}
		if (result.formattedResult.endsWith('.')) {
			result.formattedResult.chop(1);
		}
	}

	return result;
}

} // namespace Rabbit
