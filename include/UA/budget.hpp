#pragma once

namespace UA::budget::common {

enum  label_e : size_t { REP_PERIOD = 0 }; // e - enum

} // namespace UA::budget::common


namespace UA::budget {

enum  type_e : short { CHAR_, UINT32_, UINT64_, DATE_/*YM*/, STRING_/*, TEXT_*/, MONEY_/*, PERCENT_*/, TYPE_COUNT_ }; // enum of types present in budget .csv tables

} // namespace UA::budget


// Validate equality of `REP_PERIOD` value for each budget kind (incomes, expanses, ..)
#define ASSERT_UA_BUDGET_REP_PERIOD(ENUM_IDN) static_assert(static_cast<size_t>(ENUM_IDN) == static_cast<size_t>(UA::budget::common::REP_PERIOD))

// Stash //

//using type_t = std::tuple<std::chrono::year_month, char, uint64_t, uint32_t, std::string, money_t, money_t, money_t>; // T - type (tuple)
