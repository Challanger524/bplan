#pragma once

#include <filesystem>
#include <ostream>
#include <format>
#include <chrono> // time formatters
#include <string>
#include <stdint.h>
//#include <math.h>

namespace bplan::fs { // contains <filesystem> extensions


// Human-readable time formatting (without miliseconds part like: 12:12:12.12345)
inline std::string to_string(std::filesystem::file_time_type v) { return std::format("{:%F %R}:{:.2}", v, std::format("{:%S}", v)); }

// Human-readable size formatting
inline std::string filesize(uintmax_t fsize)
{
	constexpr auto BYTES_PK = 1'024; // bytes per kilobyte  // 1024: Win, Linux; 1000: mac os weiredo
	constexpr std::array powers = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB" }; // of file sizes

	short  exponent = 0;
	auto   mantissa = static_cast<long double>(fsize);
	while (mantissa >= BYTES_PK) { mantissa /= BYTES_PK; /*and*/ exponent++; }
	//mantissa = ceill(mantissa * 10) / 10; //??

	return std::format("{:.1f} {} ({})", mantissa, powers[exponent], fsize);
}
// https://en.cppreference.com/w/cpp/filesystem/file_size
// https://stackoverflow.com/questions/63512258/how-can-i-print-a-human-readable-file-size-in-c-without-a-loop
// https://github.com/eudoxos/bytesize

} // namespace bplan::fs


// Ostream custom overloads

namespace bplan::ostream {


// Human-readable time formatting (without miliseconds part like: 12:12:12.12345)
inline std::ostream &operator<<(std::ostream &os, const std::filesystem::file_time_type v) { return os << bplan::fs::to_string(v); }


} // namespace bplan::ostream
