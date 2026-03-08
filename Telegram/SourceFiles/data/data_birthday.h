/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

class Birthday final {
public:
	constexpr Birthday() = default;
	Birthday(int day, int month, int year = 0);

	[[nodiscard]] static Birthday FromSerialized(int value);
	[[nodiscard]] int serialize() const;

	[[nodiscard]] bool valid() const;

	[[nodiscard]] int day() const;
	[[nodiscard]] int month() const;
	[[nodiscard]] int year() const;

	explicit operator bool() const {
		return valid();
	}

	friend inline constexpr auto operator<=>(Birthday, Birthday) = default;
	friend inline constexpr bool operator==(Birthday, Birthday) = default;

	static constexpr auto kYearMin = 1875;
	static constexpr auto kYearMax = 2100;

private:
	int _value = 0;

};

[[nodiscard]] QString BirthdayText(Birthday date, bool fullMonth = false);
[[nodiscard]] QString BirthdayCake();
[[nodiscard]] int BirthdayAge(Birthday date);
[[nodiscard]] bool IsBirthdayToday(Birthday date);
[[nodiscard]] rpl::producer<bool> IsBirthdayTodayValue(Birthday date);

} // namespace Data
