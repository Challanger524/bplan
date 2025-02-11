#pragma once

#include "int/budget/rapidcsv/sort.hpp"
#include "UA/budget.hpp"

#include <imgui.h>

#include <span>
#include <assert.h>

namespace bplan {

/** High tier budget (csv) table sorter
 * @param &csv   csv document to be sorted inplace
 * @param specs  `ImGuiTableSortSpecs` with **local** table column indexes to be multi-sorted
 * @param labelT span of types implicitly mapped to the column indexes (`string` -> `year_month`/`uint64_t`/...)
 * @param labelV span of subset  of **real** column indexes used for viewing ImGui Table. Implicit mapping: imgui specs -> `labelV` -> real `idx`
 * @param LabelI (optional) span of **all** csv table column indexes. Activates implicit budget primary sorting by date (added as first multisort option). */
inline void SortBudget(rapidcsv::Document& csv, const ImGuiTableSortSpecs *specs, std::span<const UA::budget::type_e> labelT, std::span<const size_t> labelV, std::span<const size_t> LabelI = std::span<const size_t, 0>())
{
	assert(specs != nullptr);
	if    (specs == nullptr) return;

	std::span<const ImGuiTableColumnSortSpecs> specsSpan(specs->Specs, specs->SpecsCount);

	bp::sort_specs_t specsSort;
	specsSort.reserve(specsSpan.size() + (LabelI.empty() ? 0 : 1));

	// add implicit first spec sort - date
	if (!LabelI.empty()) specsSort.push_back(bp::SortSpec{.column = LabelI[UA::budget::common::REP_PERIOD], .ascend = true});

	// fill `bp::SortSpecs` based on ImGui `ImGuiTableSortSpecs` with real indexes from `labelV` (that is used for table creation)
	for (size_t i = 0; i < specsSpan.size(); i++)
		specsSort.push_back(bp::SortSpec {.column = labelV[specsSpan[i].ColumnIndex],
		                                  .ascend =        specsSpan[i].SortDirection == ImGuiSortDirection_Ascending});

	SortBudget(csv, specsSort, labelT);
}

} // namespace bplan
