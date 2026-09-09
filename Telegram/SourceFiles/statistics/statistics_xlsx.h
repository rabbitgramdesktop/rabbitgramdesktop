/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Statistic::Xlsx {

struct Cell final {
	enum class Type : uchar {
		Empty,
		Text,
		Header,
		Number,
		Date,
		DateTime,
	};

	Type type = Type::Empty;
	QString text;
	float64 number = 0.;
};

[[nodiscard]] Cell Text(const QString &text);
[[nodiscard]] Cell Header(const QString &text);
[[nodiscard]] Cell Number(float64 value);

[[nodiscard]] Cell Date(float64 milliseconds);
[[nodiscard]] Cell DateTime(float64 milliseconds);

struct Sheet final {
	QString name;
	std::vector<std::vector<Cell>> rows;
};

[[nodiscard]] QByteArray Serialize(const std::vector<Sheet> &sheets);

} // namespace Statistic::Xlsx
