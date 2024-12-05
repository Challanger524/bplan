#include "test.hpp"
#include "test/tests.hpp"

//#include "int/locale.hpp"

#include <rapidcsv.h>
#include <imgui.h>

#include <span>
//#include <tuple>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <locale>
#include <assert.h>

namespace bplan {
namespace chrono {
auto get(std::chrono::year  p) { return p.operator          int(); }
auto get(std::chrono::month p) { return p.operator unsigned int(); }
auto get(std::chrono::day   p) { return p.operator unsigned int(); }

auto& from_stream(const auto &idate, auto format, auto &odate) { std::istringstream sstream(idate); return std::chrono::from_stream(sstream, format, odate); }
}

template<class T> std::enable_if_t<!std::is_same_v<T, std::string>, short> Delta(T l, T r) { return l == r ? 0 : (l < r) ? -1 : +1; }
template<class T> short Delta              (const std::string & , const std::string & ) { static_assert(false, "custom template specialization required for given type" ); assert(false); return {}; }
// `std::string` that can hold `utf-8` encoding
  template<>        short Delta<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 : std::locale().operator()(l, r) ? -1 : +1;  } //! bad == cmp
//template<>        short Delta<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 :  bp::locale  .operator()(l, r) ? -1 : +1;  }
//template<>        short Delta<std::wstring>(const std::string &l, const std::string &r) { return l == r ? 0 :      locale  .operator()(l, r) ? -1 : +1;  }
//template<>        short Delta<std::string >(const std::string &l, const std::string &r) { return scast<short>(         l.compare  (r));       }
//template<>        short Delta<char        >(const std::string &l, const std::string &r) { return Delta(l.empty() ? '\0' : l[0], r.empty() ? '\0' : r[0]); }

template<>        short Delta<uint64_t    >(const std::string &l, const std::string &r) { return Delta(std::stoull(l), std::stoull(r));       }
template<>        short Delta<ldouble     >(const std::string &l, const std::string &r) { return Delta(std::stold (l), std::stold (r));       }
template<class Date> short Delta           (const std::string &l, const std::string &r, auto fromat) { // `Date` suitable for: std::chrono:: `day`, `year_month`, ...
	Date dl;
	Date dr;
	//std::istringstream sstream;
	//sstream.clear(); sstream.str(l); sstream >> std::chrono::parse(fromat, dl);
	//sstream.clear(); sstream.str(r); sstream >> std::chrono::parse(fromat, dr);
	//sstream.clear(); sstream.str(l); std::chrono::from_stream(sstream, fromat, dl);
	//sstream.clear(); sstream.str(r); std::chrono::from_stream(sstream, fromat, dr);
	bp::chrono::from_stream(l, fromat, dl);
	bp::chrono::from_stream(r, fromat, dr);
	return Delta(dl, dr);
}

// STL wrapper that accepts `container` instead of `begin()`, `end()` range iterators, YAY
constexpr auto sort(auto &container, auto comparator = std::less<>{}) { return std::sort(container.begin(), container.end(), comparator); }
} // namespace bplan

//template <class T> using undtp = class std::underlying_type_t<T>;

enum  type_e : short { CHAR, UINT32, UINT64, DATE/*YM*/, STRING/*, TEXT*/, MONEY/*, PERCENT*/, TYPE_COUNT_ }; // enum of types (across budget CSVs)
//using labelT = std::tuple               < std::chrono::year_month ,     char ,   uint64_t , uint32_t ,std::string, money_t,   money_t ,  money_t>; // T - type (tuple)
enum  labe_e : size_t                                  { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT, LABEL_COUNT_ }; // E - enum
constexpr std::array<    size_t , LABEL_COUNT_> labelP { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT /*, PERCENT*/ }; // P - pos
constexpr std::array<    type_e , LABEL_COUNT_> labelT { DATE/*YM*/ ,     CHAR ,     UINT64 ,   UINT32 ,   STRING ,   MONEY ,     MONEY ,    MONEY /*, PERCENT*/ }; // T - type
constexpr std::array<const char*, LABEL_COUNT_> labelS {"REP_PERIOD","FUND_TYP","COD_BUDGET","COD_INCO","NAME_INC","ZAT_AMT","PLANS_AMT","FAKT_AMT"/*, DCORRYP*/ }; // S - str
constexpr std::array<const char*, LABEL_COUNT_> labelD { // D - description
	/*         REP_PERIOD */ "Місяць та рік",
	/*           FUND_TYP */ "Тип фонду",
	/*         COD_BUDGET */ "Код бюджету",
	/*         COD_INCO   */ "Код доходу",
	/*           NAME_INC */ "Найменування коду доходу",
	/*            ZAT_AMT */ "Розпис на рік з урахуванням змін",
	/*          PLANS_AMT */ "Кошторисні призначення на рік з урахуванням змін",
	/*           FAKT_AMT */ "Виконано за період",
	/* DONE_CORR_YEAR_PCT */ //"Виконання до уточненого річного розпису, %", // DCORRYP - DONE_CORR_YEAR_PCT
};

namespace bplan
{
struct SortSpec {
	size_t column{};
	bool   ascend{true};
};

using sort_specs_t = std::vector<SortSpec>;
using     vecstr_t = std::vector<std::string>;

void SortBudget(std::vector<vecstr_t> &table, std::span<const SortSpec>  specs, std::span<const type_e> labelT)
{
	/*
	std::sort(table.begin(), table.end(), [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs)*/
	 bp::sort(table                     , [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs){
		for (const auto &spec : specs)
		{
			//if (lhs[column] == rhs[column]) continue; // easy to check any values for equality (coz string), but rare and resource consuming case

			short delta{};
			const auto    &column = spec.column; // real column `idx` from the main csv storage
			switch (labelT[column])
			{
				case UINT32   :
				case UINT64   : delta = bp::Delta<               uint64_t>(lhs[column], rhs[column]); break;
				case MONEY    : delta = bp::Delta<                money_t>(lhs[column], rhs[column]); break;
				case CHAR     :
				case STRING   : delta = bp::Delta<std::            string>(lhs[column], rhs[column]); break;
				case DATE     : delta = bp::Delta<std::chrono::year_month>(lhs[column], rhs[column], "%m.%Y"); break;
				default       : assert(false && "individual switch's case option for each 'type_e' required"); break;
			}

			if (delta == 0 ) continue; // compare with another column specified in next `spec` : specs
			if (spec.ascend) return delta < 0;
			else             return delta > 0;
		}

		return false; // means all Specs are equal and no need to sort
	});
}
void SortBudget(rapidcsv::Document    &  csv, std::span<const SortSpec>  specs, std::span<const type_e> labelT)
{
	const size_t csvSize [[maybe_unused]] = csv.GetRowCount();

	std::vector<bp::vecstr_t> table; // moved csv table
	table.reserve(csvSize);

	while (csv.GetRowCount() > 0) { // "move" all data from csv
		table.push_back(csv.GetRow<std::string>(0));
		csv.RemoveRow(0);
	}

	bp::SortBudget(table, specs, labelT);

	//for (const auto &ch : table) { csv.InsertRow<std::string>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::remove_const_t<std::remove_pointer_t<decltype(ch.data())>>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::decay_t<decltype(*std::begin(ch))>>(row++, ch); } // dereferencing, not good
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(ch.begin())>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(std::begin(ch))>>(row++, ch); } // final leap of solution evolution
	for (size_t i = 0; i < table.size(); i++) { csv.InsertRow<std::string>(i, table[i]); }

	assert(csvSize == csv.GetRowCount());
}
void SortBudget(rapidcsv::Document    &  csv, const ImGuiTableSortSpecs *specs, std::span<const type_e> labelT, std::span<const size_t> labelV, std::span<const size_t> LabelIforDateSort = std::span<const size_t, 0>())
{
	assert(specs != nullptr);
	std::span<const ImGuiTableColumnSortSpecs> specsSpan(specs->Specs, specs->SpecsCount);

	bp::sort_specs_t specsSort;
	specsSort.reserve(specsSpan.size() + (LabelIforDateSort.empty() ? 0 : 1));

	if (!LabelIforDateSort.empty()) specsSort.push_back(bp::SortSpec{.column = LabelIforDateSort[REP_PERIOD], .ascend = true});
	for (size_t i = 0; i < specsSpan.size(); i++)
		specsSort.push_back(bp::SortSpec {.column = labelV[specsSpan[i].ColumnIndex],
							              .ascend =        specsSpan[i].SortDirection == ImGuiSortDirection_Ascending});

	SortBudget(csv, specsSort, labelT);
}

} // namespace bplan

void Test::CsvTable()
{
	using std::string;

	if (this->csv.GetRowCount() == 0)
		test::CsvFilterT(&this->csv);

	//size_t REP_PERIOD = -1, COD_INCO = -1, NAME_INC = -1, ZAT_AMT = -1, PLANS_AMT = -1, FAKT_AMT = -1; // declaring each variable separately design
	std::array<size_t, LABEL_COUNT_> labelI; // I - id positions
	labelI.fill(std::underlying_type_t<labe_e>(-1));

	// Map header Indexes
	const auto & headers = this->csv.GetColumnNames();
	for (size_t i = 0; i < headers.size(); i++)
		for (size_t j = 0; j < labelS.size(); j++)
			if (headers[i] == labelS[j]) labelI[j] = i;

	// assert presence of all required headers //? replace assert with runtime error/exception
	for (size_t i = 0; i < labelS.size(); i++)
		assert(labelI[i] != std::underlying_type_t<labe_e>(-1));

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
	im::Begin(STR(TEST_VARIANT_CSV_MONO), &this->showtestB[this->CSV_TABLE], flagsWindow);

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
#endif
#if defined(TEST_VARIANT_CSV_TREE)   || 0 // 1 table with dates tree breakdown
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TREE), &this->showtestB[this->CSV_TABLE], flagsWindow);
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
#endif
#if defined(TEST_VARIANT_CSV_TABLED) || 0 // table of tables breakdown
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TABLED), &this->showtestB[this->CSV_TABLE], flagsWindow);
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
#endif
#if defined(TEST_VARIANT_CSV_TABBED) || 0 // Tabbed tables - (tooltips, synced), date breaks with tabs
	windowTestNextPos += windowTestPosDiff;
	im::SetNextWindowPos(windowTestNextPos, ImGuiCond_FirstUseEver);
	im::SetNextWindowContentSize(windowTestSize);
	im::Begin(STR(TEST_VARIANT_CSV_TABBED), &this->showtestB[this->CSV_TABLE], flagsWindow);

	constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

	std::chrono::year_month period; // holds REP_PERIOD (converted)
	std::istringstream sstream(csv.GetCell<string>(labelI[REP_PERIOD], 0)); // holds REP_PERIOD (raw)
	sstream >> std::chrono::parse("%m.%Y", period);
	//std::chrono::from_stream(sstream, "%m.%Y", period);

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
			sstream.clear(); // `.str()` not clearing the state, 4uck you responsible STL vendors/commies
			sstream.str(timePeriod);
			sstream >> std::chrono::parse("%m.%Y", period);
			//std::chrono::from_stream(sstream, "%m.%Y", period);

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
#endif
}
