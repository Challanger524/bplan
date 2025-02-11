#pragma once

#include "UA/budget.hpp"
#include "bplan/chrono.hpp"
#include "bplan/compare.hpp"

#include <span>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>   // sort (wrapper)
#include <type_traits>
#include <assert.h>

namespace bplan {

// STL wrapper that accepts `container` instead of `container.begin()`, `container.end()` iterators
constexpr inline auto sort(auto &container, auto comparator = std::less<>{}) { return std::sort(container.begin(), container.end(), comparator); }

struct SortSpec {
	size_t column{};
	bool   ascend{true};
};

using sort_specs_t = std::vector<SortSpec>;
using     vecstr_t = std::vector<std::string>;

/** Low tier budget table sorter
 * @param &table two-dimensional table of strings to be sorted inplace
 * @param specs  span of **real** table column indexes to be multi-sorted
 * @param labelT span of types implicitly mapped to the column indexes (string -> year_month/uint64_t/..) */
inline void SortBudget(std::vector<vecstr_t> &table, std::span<const SortSpec> specs, std::span<const UA::budget::type_e> labelT)
{
	using enum UA::budget::type_e;
	/*
	std::sort(table.begin(), table.end(), [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs)*/
	 bp::sort(table                     , [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs)
	{ // lambda body start
		for (const auto &spec : specs)
		{
			short         comp{};               // compare result
			const size_t &column = spec.column; // real column `idx` from the main csv storage
			const auto   &type = labelT[column];
			switch (      type)
			{
				case UINT32_:
				case UINT64_: comp = bp::compare<               uint64_t>(lhs[column], rhs[column]); break;
				case  MONEY_: comp = bp::compare<                money_t>(lhs[column], rhs[column]); break;
				case   CHAR_:
				case STRING_: comp = bp::compare<std::            string>(lhs[column], rhs[column]); break;
				case   DATE_: comp = bp::compare<std::chrono::year_month>(lhs[column], rhs[column], "%m.%Y"); break;
				default     : assert(false && "individual switch case for each 'type_e' required" ); break;
			}

			if (comp == 0  ) continue; // skip this, but compare another column specified in next `spec` : specs
			if (spec.ascend) return comp < 0;
			else             return comp > 0;
		}

		return false; // means all `specs` are equal and no need to sort

	} /* lambda body end */ ); // sort()

}

} // namespace bplan


// Stash //

			//if (lhs[column] == rhs[column]) continue; // easy to check any values for equality (coz string), but rare and resource consuming case
