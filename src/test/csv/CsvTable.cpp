#include "test/csv/CsvTable.hpp"
#include "test/tests.hpp"

#include "int/budget/rapidcsv/imgui/sort.hpp"
#include "UA/budget.hpp"
#include "UA/budget/incomes.hpp"
#include "bplan/chrono.hpp"

#include <imgui.h>

#include <array>
#include <string>
#include <chrono>
#include <locale>
#include <type_traits>
#include <assert.h>

using namespace UA::budget;
using namespace UA::budget::incomes;
//namespace bud = UA::budget;
//namespace inc = UA::budget::incomes;

namespace test {

CsvTable::CsvTable()
{
	test::CsvFilterT(&this->csv);

	labelI.fill(std::underlying_type_t<label_e>(-1));

	// Map header Indexes
	const auto & headers = this->csv.GetColumnNames();
	for (size_t i = 0; i < headers.size(); i++)
		for (size_t j = 0; j < labelS.size(); j++)
			if (headers[i] == labelS[j]) labelI[j] = i;

	// assert presence of all required headers //? replace assert with runtime error/exception
	for (size_t i = 0; i < labelS.size(); i++)
		assert(labelI[i] != std::underlying_type_t<label_e>(-1));
}

void CsvTable::operator()()
{
	using std::string;

	// Create a visual ImGui table of the data

	constexpr ImGuiWindowFlags flagsWindow =
		ImGuiWindowFlags_None                  |
		ImGuiWindowFlags_NoCollapse            |
		ImGuiWindowFlags_NoSavedSettings       |
		ImGuiWindowFlags_AlwaysAutoResize      |
		0;

	constexpr ImGuiTableFlags flagsTable =
		ImGuiTableFlags_NoSavedSettings        |
		ImGuiTableFlags_Reorderable            |
		ImGuiTableFlags_Resizable              |
		ImGuiTableFlags_Hideable               |
		ImGuiTableFlags_ScrollY                |
		ImGuiTableFlags_ScrollX                |
		ImGuiTableFlags_Borders                |
		ImGuiTableFlags_RowBg                  |
		ImGuiTableFlags_Sortable               |
		ImGuiTableFlags_SortMulti              |
		ImGuiTableFlags_SortTristate           |
		ImGuiTableFlags_SizingFixedFit         |
		//ImGuiTableFlags_SizingStretchProp      |
		//ImGuiTableFlags_HighlightHoveredColumn |
		0;

	constexpr ImVec2 windowTestSize   ( 0.f, 400.f);
	constexpr ImVec2 windowTestPosDiff(30.f,  30.f);
	          ImVec2 windowTestNextPos( 0.f,   0.f);

  #define TEST_VARIANT_CSV_MONO   // Testing: mono table (tooltips), without date breaks
  #define TEST_VARIANT_CSV_TREE   // 1 table with dates tree breakdown
  #define TEST_VARIANT_CSV_TABLED // table of tables breakdown
  #define TEST_VARIANT_CSV_TABBED // Tabbed tables - (tooltips, synced), date breaks with tabs

#if defined(TEST_VARIANT_CSV_MONO)   || 0 // Testing: mono table (tooltips), without date breaks
	windowTestNextPos += windowTestPosDiff; //? no use outside of "FirstUseEver" since it will be incremented on each iteration (of the main loop)
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_MONO), nullptr, flagsWindow);

	const std::array labelV { labelI[REP_PERIOD], labelI[COD_INCO], labelI[ZAT_AMT], labelI[FAKT_AMT], labelI[PLANS_AMT] }; // V - columns to view
	if (im::BeginTable("csv", scast<int>(labelV.size()), flagsTable))
	{
		{ // declare column headers
			size_t c = 0;
			im::TableSetupScrollFreeze(0, 1);
			im::TableSetupColumn(labelS[labelV[c++]] /* REP_PERIOD */, ImGuiTableColumnFlags_NoHeaderWidth /*| ImGuiTableColumnFlags_NoHeaderLabel*/);
			im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO */);
			im::TableSetupColumn(labelS[labelV[c++]] /*    ZAT_AMT */);
			im::TableSetupColumn(labelS[labelV[c++]] /*   FAKT_AMT */);
			im::TableSetupColumn(labelS[labelV[c++]] /*  PLANS_AMT */, ImGuiTableColumnFlags_DefaultHide);
		}

		// Draw table header row
		im::TableNextRow(ImGuiTableRowFlags_Headers);
		const int columns_count = im::TableGetColumnCount();
		for (int c = 0; c < columns_count; c++)
		{
			if (!im::TableSetColumnIndex(c)) continue;

			const char *name = im::TableGetColumnName(c);
			im::PushID(c);
			im::TableHeader(name);
			im::PopID();

			if (im::IsItemHovered()) {
				im::BeginTooltip();
				im::Text("%s", labelD[labelV[c]]); // tooltip with table header label description
				im::EndTooltip();
			}

		}

		/*
		im::TableSetupColumn(labelS[COD_INCO ]);
		im::TableSetupColumn(labelS[ZAT_AMT  ]);
		im::TableSetupColumn(labelS[FAKT_AMT ]);
		im::TableSetupColumn(labelS[PLANS_AMT], ImGuiTableColumnFlags_DefaultHide);
		im::TableHeadersRow(); */

		/*
		for (const auto c : labelV)
			im::TableSetupColumn(labelS[c]);
		im::TableHeadersRow();*/

		/*
		im::TableSetupColumn(labelS[labelV[0]]);
		im::TableSetupColumn(labelS[labelV[1]]);
		im::TableSetupColumn(labelS[labelV[2]]);
		im::TableSetupColumn(labelS[labelV[3]], ImGuiTableColumnFlags_DefaultHide);
		im::TableHeadersRow();*/

		/*
		ImGuiListClipper clipper;
		clipper.Begin(scast<int>(this->csv.GetRowCount()));
		while (clipper.Step()) {
			for (size_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {*/

		// check and handle sorting request
		if (ImGuiTableSortSpecs *specs = im::TableGetSortSpecs(); specs && specs->SpecsDirty) {
			bp::SortBudget(this->csv, specs, labelT, labelV);
			specs->SpecsDirty = false;
		}

		// setup clipper that will draw only necessery table rows (not all the rows)
		size_t rowCount = this->csv.GetRowCount();
		ImGuiListClipper clipper;
		clipper.Begin(scast<int>(rowCount) /*, im::GetTextLineHeight()*/);

		while (clipper.Step())
		{
			//for (size_t row = 0; row < rowCount; row++) // iterating without `clipper` - example
			for (size_t row = scast<size_t>(clipper.DisplayStart); row < scast<size_t>(clipper.DisplayEnd); row++)
			{
				size_t c = 0;
				im::TableNextRow(/*ImGuiTableRowFlags_None, im::GetTextLineHeight()*/);
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /* REP_PERIOD */, row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*   COD_INCO */, row).c_str()); /*scope*/ { // tooltip
					if (im::IsItemHovered()) {
						im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
						im::Text("%s", this->csv.GetCell<string>(labelI[NAME_INC], row).c_str());
						im::PopTextWrapPos(); /*and*/ im::EndTooltip();
					}
				}
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*   FAKT_AMT */, row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*    ZAT_AMT */, row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*  PLANS_AMT */, row).c_str());

				/*
				for (const auto col : labelV) {
					im::TableNextColumn();
					im::TextUnformatted(this->csv.GetCell<std::string>(col, row).c_str());
				}*/
			}
		}

		im::EndTable();
	}

	im::End();
#endif   // TEST_VARIANT_CSV_MONO
#if defined(TEST_VARIANT_CSV_TREE)   || 0 // 1 table with dates tree breakdown
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TREE), nullptr, flagsWindow);
	if (im::BeginTable("table tree", 4, flagsTable ^ ImGuiTableFlags_Sortable, {300.f, 0.f}))
	{
		im::TableSetupScrollFreeze(0, 1);
		im::TableSetupColumn(labelS[ COD_INCO], ImGuiTableColumnFlags_NoHeaderWidth);
		im::TableSetupColumn(labelS[  ZAT_AMT]);
		im::TableSetupColumn(labelS[ FAKT_AMT]);
		im::TableSetupColumn(labelS[PLANS_AMT], ImGuiTableColumnFlags_DefaultHide);
		im::TableHeadersRow();

		std::string timePeriod;
		bool timePeriodOpen = false;
		for (int row = 0; row < scast<int>(this->csv.GetRowCount()); row++)
		{
			// spot the month change
			if (timePeriod != this->csv.GetCell<string>(labelI[REP_PERIOD], row)) {
				timePeriod  = this->csv.GetCell<string>(labelI[REP_PERIOD], row);

				im::TableNextRow();
				im::TableNextColumn();

				if (timePeriodOpen) {
					im::TreePop();
					im::Indent();
				}

				timePeriodOpen = im::TreeNodeEx(timePeriod.c_str(), ImGuiTreeNodeFlags_SpanAllColumns /* | ImGuiTreeNodeFlags_DefaultOpen */);
				if (timePeriodOpen) {
					im::Unindent();
				}
			}

			if (timePeriodOpen) {
				im::TableNextRow();
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[ COD_INCO], row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[  ZAT_AMT], row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[ FAKT_AMT], row).c_str());
				im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[PLANS_AMT], row).c_str());
			}
		}

		if (timePeriodOpen)
			im::TreePop();

		im::EndTable();
	}

	im::End();
#endif   // TEST_VARIANT_CSV_TREE
#if defined(TEST_VARIANT_CSV_TABLED) || 0 // table of tables breakdown
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TABLED), nullptr, flagsWindow);
	if (im::BeginTable("table tables", 1, flagsTable ^ ImGuiTableFlags_Sortable, windowTestSize))
	{
		im::TableSetupScrollFreeze(0, 1);
		im::TableSetupColumn(labelS[REP_PERIOD], ImGuiTableColumnFlags_NoHeaderWidth, 300.f);
		im::TableHeadersRow();

		std::string timePeriod;
		bool timePeriodOpen = false;
		for (int row = 0; row < scast<int>(this->csv.GetRowCount()); row++)
		{
			// spot the month change
			if (timePeriod != this->csv.GetCell<string>(labelI[REP_PERIOD], row)) {
				timePeriod  = this->csv.GetCell<string>(labelI[REP_PERIOD], row);

				im::TableNextRow();
				im::TableNextColumn();

				if (timePeriodOpen) im::TreePop();
				timePeriodOpen = im::TreeNodeEx(timePeriod.c_str(), ImGuiTreeNodeFlags_SpanAllColumns /* | ImGuiTreeNodeFlags_DefaultOpen */);

				if (timePeriodOpen)
				{
					//im::Unindent();
					if (im::BeginTable("csv", 4, flagsTable ^ ImGuiTableFlags_Sortable))
					{
						im::TableSetupScrollFreeze(0, 1);
						im::TableSetupColumn(labelS[ COD_INCO], ImGuiTableColumnFlags_NoHeaderWidth);
						im::TableSetupColumn(labelS[  ZAT_AMT]);
						im::TableSetupColumn(labelS[ FAKT_AMT]);
						im::TableSetupColumn(labelS[PLANS_AMT], ImGuiTableColumnFlags_DefaultHide);
						im::TableHeadersRow();

						for (/*row*/; row < scast<int>(this->csv.GetRowCount()); row++){
							if (timePeriod != this->csv.GetCell<string>(labelI[REP_PERIOD], row)) { row--; break;}

							im::TableNextRow();
							im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[ COD_INCO], row).c_str());
							im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[  ZAT_AMT], row).c_str());
							im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[ FAKT_AMT], row).c_str());
							im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelI[PLANS_AMT], row).c_str());
						}

						im::EndTable();
					}
				}
			}

			if (timePeriodOpen) {
			}
		}

		if (timePeriodOpen)
			im::TreePop();

		im::EndTable();
	}

	im::End();
#endif   // TEST_VARIANT_CSV_TABLED
#if defined(TEST_VARIANT_CSV_TABBED) || 0 // Tabbed tables - (tooltips, synced), date breaks with tabs
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TABBED), nullptr, flagsWindow);

	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	std::chrono::year_month period; // holds REP_PERIOD (converted)
	auto timePeriod = csv.GetCell<string>(labelI[REP_PERIOD], 0);
	bp::chrono::from_stream(timePeriod, "%m.%Y", period);

	//im::Text("%d:", period.year().operator int()); /*and*/ im::SameLine(); // display `year` as text
	if (im::BeginTabBar("csvTabBar", flagsTabBar))
	{
		// Display `year` as leading tab (button)
		im::TabItemButton(std::format("{}:", bp::chrono::get(period.year())).c_str(), ImGuiTabItemFlags_Leading  /*| ImGuiTabItemFlags_NoTooltip*/);
		//im::TabItemButton(          "(month/quarter)"                             , ImGuiTabItemFlags_Trailing /*| ImGuiTabItemFlags_NoTooltip*/);

		size_t countPeriod = 0;
		std::string timePeriod;
		//const ImGuiID id = im::GetID("table csv");

		const std::array labelV { labelI[REP_PERIOD], labelI[COD_INCO], labelI[ZAT_AMT], labelI[FAKT_AMT], labelI[PLANS_AMT], labelI[NAME_INC] }; // C - columns to show
		const size_t rowCount = this->csv.GetRowCount();

		for (size_t row = 0; row < rowCount; /*++*/) // display months/quarters as tabs with a table (with recpective data part)
		{
			timePeriod = this->csv.GetCell<string>(labelI[REP_PERIOD], row);
			bp::chrono::from_stream(timePeriod, "%m.%Y", period);

			//const bool beginTabItem = im::BeginTabItem(std::to_string(bp::chrono::get(period.month())).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			const bool beginTabItem = im::BeginTabItem(std::format("{}##{}", bp::chrono::get(period.month()), countPeriod).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
			/*
			im::SetItemTooltip("%s", std::format(std::locale("uk_UA.utf8"), "{:L%B}", period.month()).c_str());*/ /*
			im::SetItemTooltip("%s", std::format( bp::locale              , "{:L%B}", period.month()).c_str());*/
			im::SetItemTooltip("%s", std::format(                           "{:L%B}", period.month()).c_str());
			if (beginTabItem)
			{
				//im::PushOverrideID(id); //internal// sync all tables across tabs (showing columns, sizes, scroll level, sort)
				if (im::BeginTable("csv table", scast<int>(labelV.size()), flagsTable | ImGuiTableFlags_Sortable))
				{
					{ // declare column headers
						size_t c = 0;
						im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
						im::TableSetupColumn(labelS[labelV[c++]] /* REP_PERIOD */, ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_NoSort    /*| ImGuiTableColumnFlags_DefaultHide*/);
						im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO */, ImGuiTableColumnFlags_NoHeaderWidth | ImGuiTableColumnFlags_DefaultSort);
						im::TableSetupColumn(labelS[labelV[c++]] /*   FAKT_AMT */);
						im::TableSetupColumn(labelS[labelV[c++]] /*    ZAT_AMT */);
						im::TableSetupColumn(labelS[labelV[c++]] /*  PLANS_AMT */, ImGuiTableColumnFlags_DefaultHide);
						im::TableSetupColumn(labelS[labelV[c++]] /*   NAME_INC */, ImGuiTableColumnFlags_DefaultHide);
					}

					// check and handle sorting request
					if (ImGuiTableSortSpecs *spec = im::TableGetSortSpecs(); spec && spec->SpecsDirty) {
						bp::SortBudget(this->csv, spec, labelT, labelV, labelI);
						spec->SpecsDirty = false;
					}

					// draw column headers (with tooltips)
					im::TableNextRow(ImGuiTableRowFlags_Headers);
					const int columns_count = im::TableGetColumnCount();
					for (int c = 0; c < columns_count; c++)
					{
						if (!im::TableSetColumnIndex(c)) continue;

						const char *const name = im::TableGetColumnName(c);
						im::PushID(c); /*and*/ im::TableHeader(name); /*and*/ im::PopID();

						// tooltip with label description
						if (im::IsItemHovered()) {
							im::BeginTooltip(); /*and>*/ im::Text("%s", labelD[labelV[c]]); /*<and*/ im::EndTooltip();
						}
					}

					// draw rows
					for (/*row*/; row < rowCount && timePeriod == this->csv.GetCell<string>(labelI[REP_PERIOD], row); row++)
					{
						size_t c = 0;
						im::TableNextRow();
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /* REP_PERIOD */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*   COD_INCO */, row).c_str()); /*scope*/ { // tooltip
							if (im::IsItemHovered()) {
								im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
								im::Text("%s", this->csv.GetCell<string>(labelI[NAME_INC], row).c_str());
								im::PopTextWrapPos(); /*and*/ im::EndTooltip();
							}
						}
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*   FAKT_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*    ZAT_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*  PLANS_AMT */, row).c_str());
						im::TableNextColumn(); im::TextUnformatted(this->csv.GetCell<string>(labelV[c++] /*   NAME_INC */, row).c_str());
					}

					im::EndTable();
				}
				else while (++row < rowCount && timePeriod == this->csv.GetCell<string>(labelI[REP_PERIOD], row));

				//im::PopID(); // closure of: im::PushOverrideID(id)
				im::EndTabItem();
			}
			else while (++row < rowCount && timePeriod == this->csv.GetCell<string>(labelI[REP_PERIOD], row));

			countPeriod++;

		}

		im::EndTabBar();
	}

	im::End();
#endif   // TEST_VARIANT_CSV_TABBED
}

} // namespace test
