#pragma once

#include <filesystem>
#include <ostream>
#include <format>
#include <chrono> // time formatters
#include <string>

namespace bplan::fs { // contains <filesystem> extensions


// Human-readable time formatting (without miliseconds part like: 12:12:12.12345)
inline std::string  to_string  (                  const std::filesystem::file_time_type v) { return std::format("{:%F %R}:{:.2}", v, std::format("{:%S}", v)); }


} // namespace bplan::fs


// Ostream custom overloads

namespace bplan::ostream {


// Human-readable time formatting (without miliseconds part like: 12:12:12.12345)
inline std::ostream &operator<<(std::ostream &os, const std::filesystem::file_time_type v) { return os << bplan::fs::to_string(v); }


} // namespace bplan::ostream
