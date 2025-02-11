#pragma once

#include "int/budget/sort.hpp"
#include "UA/budget.hpp"

#include <rapidcsv.h>

#include <span>
#include <vector>
#include <string>
#include <assert.h>

namespace bplan {

/** Mid tier budget (csv) table sorter
 * @param &csv csv document to be sorted inplace
 * @param specs  span of **real** table column indexes to be multi-sorted
 * @param labelT span of types implicitly mapped to the column indexes (string -> year_month/uint64_t/..) */
inline void SortBudget(rapidcsv::Document& csv, std::span<const SortSpec> specs, std::span<const UA::budget::type_e> labelT)
{
	const size_t csvSize [[maybe_unused]] = csv.GetRowCount();

	std::vector<bp::vecstr_t> table;  // moved csv table
	table.reserve(csv.GetRowCount()); //

	// copy-erase all data from `csv` to `vector<string> table`
	while (csv.GetRowCount() > 0) {
		table.push_back(csv.GetRow<std::string>(0));
		csv.RemoveRow(0);
	}

	bp::SortBudget(table, specs, labelT);

	for (size_t i = 0; i < table.size(); i++) csv.InsertRow<std::string>(i, table[i]);

	assert(csvSize == csv.GetRowCount());
}

} // namespace bplan


// Stash //

	//for (const auto &ch : table) { csv.InsertRow<std::string>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::remove_const_t<std::remove_pointer_t<decltype(ch.data())>>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::decay_t<decltype(*std::begin(ch))>>(row++, ch); } // dereferencing, not good
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(ch.begin())>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(std::begin(ch))>>(row++, ch); }
