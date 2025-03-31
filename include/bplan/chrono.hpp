#pragma once

#include <chrono>

#if defined(__clang__)
#  include <string>
#  include <sstream>
#  include <type_traits>
#  include <assert.h>
#endif

namespace bplan::chrono {

// Unified `get(<y/m/d>)` function to deal with different return types of `explicit (cast) operator` -> `(un)signed int`
inline auto get(std::chrono::year  p) { return p.operator          int(); }
inline auto get(std::chrono::month p) { return p.operator unsigned int(); }
inline auto get(std::chrono::day   p) { return p.operator unsigned int(); }

// Clang has only particial C++20 Calendar and Time Zone implementation
// https://github.com/llvm/llvm-project/issues/99982
// https://libcxx.llvm.org/Status/Cxx20.html#note-p0355:~:text=Extending%20chrono%20to%20Calendars%20and%20Time%20Zones
#if !defined(__clang__)
// Wrapper that accepts any `idate` string that can construct `std::istringstream(idate)` - saves on `std::istringstream` creation (1 line)
inline auto& from_stream(const auto &idate, auto format, auto &odate) { std::istringstream sstream(idate); return std::chrono::from_stream(sstream, format, odate); }
#else
inline void  from_stream(const auto &idate, auto format, auto &odate) // ! Clang workaround !, only for `chrono::year_month` and only with `"%m.%Y"`
{
	constexpr std::string format_ym("%m.%Y");
	assert(               format_ym.compare(format) == 0);                         // limited    <format> support
	static_assert(std::is_same<decltype(odate), std::chrono::year_month&>::value); // limited std::<date> support

	std::istringstream iss(idate);
	int month{}, year{};

	// example: "04.2023"
	iss >> month;       // "04"
	iss.ignore(1, '.'); // '.'
	iss >> year;        // "2023"

	odate = std::chrono::year_month(std::chrono::year(year), std::chrono::month(month));
}
#endif // __clang__

// Human-readable time formatting (without miliseconds part like: 12:12:12.12345)
template<class Clock, class Duration> std::string to_string(const std::chrono::time_point<Clock, Duration>& v) { return std::format("{:%F %R}:{:.2}", v, std::format("{:%S}", v)); }

} // namespace bplan::chrono


// Stash //

	//std::istringstream sstream(<str>);
	//std::chrono::from_stream(sstream, "%m.%Y", period);
	// or:
	//sstream >> std::chrono::parse("%m.%Y", period);
