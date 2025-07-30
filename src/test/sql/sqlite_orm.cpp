#include "test/sql/sqlite_orm.hpp"

#include "UA/budget/local/incomes.hpp"
#include "bplan/chrono.hpp"

#include <sqlite_orm.h>
#include <rapidcsv.h>
#include <imgui.h>

#include <map>
#include <format>
#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <type_traits>

namespace test {


namespace rcv = rapidcsv;
using std::string;

static constexpr auto CSV_PATH = "res/testing/local_2555900000_INCOMES_2023_month.csv";
static constexpr auto DB_LOCAL = "local.sqlite";

inline auto InitStorage(const std::string &dbPathName)
{
	using namespace sqlite_orm;
	return make_storage(dbPathName
		, make_table("INCOMES"
			, make_column("COD_BUDGET", &Income::COD_BUDGET)
			, make_column("YEAR"      , &Income::GetYear , &Income::SetYear )
			, make_column("MONTH"     , &Income::GetMonth, &Income::SetMonth)
			, make_column("COD_INCO"  , &Income::COD_INCO)
			, make_column("NAME_INC"  , &Income::NAME_INC)
			, make_column(  "ZAT_AMT" , &Income::  ZAT_AMT)
			, make_column("PLANS_AMT" , &Income::PLANS_AMT)
			, make_column( "FAKT_AMT" , &Income:: FAKT_AMT)
			, foreign_key(&Income::NAME_INC).references(&IncCodeName::id)
		)
		, make_table("IncCodeNames"
			, make_column("id"      , &IncCodeName::id      , primary_key().autoincrement())
			, make_column("COD_INCO", &IncCodeName::COD_INCO,      unique())
			, make_column("NAME_INC", &IncCodeName::NAME_INC)
		)
	);
}

using Storage = decltype(InitStorage(""));

SqliteOrm::SqliteOrm() {
	try { // SqliteOrm::SqliteOrm()
		using namespace sqlite_orm;
		Storage storage = InitStorage(DB_LOCAL);
		storage.sync_schema();

#if 1 // DB re-recreation // very time consuming (on debug)
		storage.remove_all<Income     >();
		storage.remove_all<IncCodeName>();

		rcv::Document csv{};
		try { csv.Load(CSV_PATH, rcv::LabelParams(), rcv::SeparatorParams(';', false, true, true, false), rcv::ConverterParams(), rcv::LineReaderParams(false, '#', true)); }
		catch (std::exception &e) { std::cerr << "EXCEP: Failed to load .csv: " << CSV_PATH << ": " << e.what() << "\n"; throw e; }

		// Fill `IncCodeNames` DB Table with unique income 'codes' and 'code-names'
		{
			std::map<uint32_t, string> codeNameMap;

			const auto rowCount = csv.GetRowCount();
			for (size_t i = 0; i < rowCount; i++) {
				try {
					//const uint32_t COD_INCO = csv.GetCell<uint32_t>("COD_INCO", i);
					const uint32_t codeInc = std::stoul(csv.GetCell<string>("COD_INCO", i)); //~ catch
					const string   nameInc = csv.GetCell<string  >("NAME_INC", i);
					if (!codeNameMap.contains(codeInc)) codeNameMap[codeInc] = nameInc;
				}
				catch(std::exception &e) { std::cerr << e.what() << '\n'; }
			}

			[[maybe_unused]] int idc = 0; // id counter
			for (auto &cn : codeNameMap) {
				/*
				storage.insert (IncCodeName{.id =    -1, .COD_INCO = cn.first, .NAME_INC = std::move(cn.second)}); // indexed from: 1 */
				storage.replace(IncCodeName{.id = idc++, .COD_INCO = cn.first, .NAME_INC = std::move(cn.second)}); // indexed from: 0
			}

			storage.sync_schema();
		}

		// Fill `Income` DB Table and connect unique Foreign Key refs to Code Names
		{
			const auto rowCount = csv.GetRowCount();
			for (size_t i = 0; i < rowCount; i++)
			{
				if (csv.GetCell<string>("FUND_TYP", i) != "T") continue; // skip non-together entries

				Income income;

				income.COD_BUDGET = csv.GetCell<string>("COD_BUDGET", i);

				const string date = csv.GetCell<string>("REP_PERIOD", i); // like: 01.2025
				bp::chrono::from_stream(date, "%m.%Y", income.REP_PERIOD);

				income.COD_INCO = csv.GetCell<uint32_t>("COD_INCO", i);

				const auto incCodNameId = storage.select(&IncCodeName::id, where(is_equal(&IncCodeName::COD_INCO, income.COD_INCO))); //~ catch
				if (incCodNameId.size() == 1) income.NAME_INC = std::make_unique<int>(incCodNameId.front()); // foreign key (int) referencing
				else std::cerr << "WARN: incCodNameId.size() == '" << incCodNameId.size() << "', expected 1\n";

				// wrapper to intercept empty strings and return `auto(0)` in such case
				auto StoWrapper = [](auto(*std_sto)(const string&, size_t*), const string &str) -> std::invoke_result_t<decltype(std_sto), const string&, size_t*> {
					if (str.empty()) return 0; // cast to the determined return type
					else             return std_sto(str, nullptr); //~ catch
				};

				income.  ZAT_AMT = StoWrapper(std::stold, csv.GetCell<string>(  "ZAT_AMT", i));
				income.PLANS_AMT = StoWrapper(std::stold, csv.GetCell<string>("PLANS_AMT", i));
				income. FAKT_AMT = StoWrapper(std::stold, csv.GetCell<string>( "FAKT_AMT", i));

				storage.insert(income);
			}

			storage.sync_schema();
		}

#endif

		// Prepare member data
		this->codeNames = storage.get_all<IncCodeName>();
		this->months    = storage.select(distinct(&Income::GetMonth));
		std::sort(this->months.begin(), this->months.end());

		// print Code-Name table
		for (const auto &cn : this->codeNames) std::cout << storage.dump(cn) << std::endl;

		// print few freshly stored entires from DB
		constexpr ushort FIRST_FEW = 12;
		for (ushort c = 0; const auto &inc : storage.iterate<Income>()) {
			std::cout << storage.dump(inc) << std::endl;
			if (++c > FIRST_FEW) break;
		}
	}
	catch(std::exception &e) { std::cerr << e.what() << '\n'; throw e; }

}

void SqliteOrm::operator()()
{
	constexpr ImGuiWindowFlags wFlags =
	ImGuiWindowFlags_NoCollapse            |
	ImGuiWindowFlags_NoSavedSettings       |
	ImGuiWindowFlags_AlwaysAutoResize      |
	0;

	im::SetNextWindowSizeConstraints(ImVec2{0, 0}, ImVec2{FLT_MAX, 400});

	if (im::Begin("TEST_SQL_SQLITE_ORM", nullptr, wFlags))
	{
		constexpr ImGuiTabBarFlags flagsTabBar =
		ImGuiTabBarFlags_None                    |
		ImGuiTabBarFlags_NoTooltip               | // custom tooltip replaces this anyways
		ImGuiTabBarFlags_FittingPolicyScroll     | // no squash - no tooltip with full name
		ImGuiTabBarFlags_DrawSelectedOverline    | // a visual nimb over the selected tab for better differentiation
		//ImGuiTabBarFlags_Reorderable             |
		//ImGuiTabBarFlags_FittingPolicyResizeDown |
		0;

		if (im::BeginTabBar(__func__, flagsTabBar))
		{
			for (const auto month : this->months) // display months/quarters as tabs with a table (with recpective data)
			{
				const bool beginTabItem = im::BeginTabItem(std::format("{}", month).c_str(), nullptr, ImGuiTabItemFlags_NoPushId);
				im::SetItemTooltip("%s", std::format("{:L%B}", std::chrono::month(month)).c_str());

				if (beginTabItem)
				{
					// if another `month` => load new `month` 'REP_PERIOD' rows from DB
					if (this->currentMonth != month) this->incomes.clear();
					if (this->incomes.empty()) {
						using namespace sqlite_orm;
						Storage sql = InitStorage(DB_LOCAL);
						/*   */ sql.sync_schema();
						//this->   incomes = sql.get_all<Income>(where(is_equal(&Income::GetMonth, month)));
						this->     incomes = sql.get_all<Income>(where(c(&Income::GetMonth) == month), multi_order_by(order_by(&Income::COD_INCO).asc(), order_by(&Income::ZAT_AMT).desc()));
						this->currentMonth = month;
					}

					constexpr ImGuiTableFlags flagsTable =
						ImGuiTableFlags_NoSavedSettings        |
						ImGuiTableFlags_Reorderable            |
						ImGuiTableFlags_Resizable              |
						ImGuiTableFlags_Hideable               |
						ImGuiTableFlags_ScrollY                |
						ImGuiTableFlags_ScrollX                |
						ImGuiTableFlags_Borders                |
						ImGuiTableFlags_RowBg                  |
						//ImGuiTableFlags_Sortable               |
						//ImGuiTableFlags_SortMulti              |
						//ImGuiTableFlags_SortTristate           |
						ImGuiTableFlags_SizingFixedFit         | // will have minimal column width possible
						//ImGuiTableFlags_SizingStretchProp      | // will have odd width (more than cell/column needs)
						//ImGuiTableFlags_ContextMenuInBody      |
						//ImGuiTableFlags_HighlightHoveredColumn |
						0;

					using namespace UA::budget::incomes;
					static const std::array labelV {REP_PERIOD, COD_INCO, ZAT_AMT, FAKT_AMT, PLANS_AMT}; // C - columns to show
					if (im::BeginTable("sql table", scast<int>(labelV.size()), flagsTable))
					{
						{ // declare column headers
							size_t c = 0;
							im::TableSetupScrollFreeze(0, 1); // pin header (1st row)
							im::TableSetupColumn(labelS[labelV[c++]] /* REP_PERIOD */, /* ImGuiTableColumnFlags_DefaultHide, */ ImGuiTableColumnFlags_NoHeaderWidth);
							im::TableSetupColumn(labelS[labelV[c++]] /*   COD_INCO */);
							im::TableSetupColumn(labelS[labelV[c++]] /*   FAKT_AMT */);
							im::TableSetupColumn(labelS[labelV[c++]] /*    ZAT_AMT */);
							im::TableSetupColumn(labelS[labelV[c++]] /*  PLANS_AMT */, ImGuiTableColumnFlags_DefaultHide);
							assert(c == labelV.size());
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
						for (const auto &inc : this->incomes)
						{
							im::TableNextRow();
							im::TableNextColumn(); /* REP_PERIOD */ im::TextUnformatted(std::format("{}.{:0>2}", inc.GetYear(), inc.GetMonth()).c_str());
							im::TableNextColumn(); /*   COD_INCO */ im::TextUnformatted(std::to_string(inc.COD_INCO).c_str()); /*scope*/ { // tooltip
								if (im::IsItemHovered()) {
									im::BeginTooltip();   /*and*/ im::PushTextWrapPos(im::GetCursorPos().x + 600.f);
									//im::Text("%u - %s", this->codeNames[*inc.NAME_INC].COD_INCO, this->codeNames[*inc.NAME_INC].NAME_INC.c_str()); // bad to rely on index
									const auto it = std::find_if(this->codeNames.begin(),
									                             this->codeNames.  end(),
									                             [&id = *inc.NAME_INC](const IncCodeName &cn) { return cn.id == id; });
									im::Text("%u - %s", it->COD_INCO, it->NAME_INC.c_str());
									im::PopTextWrapPos(); /*and*/ im::EndTooltip();
								}
							}
							im::TableNextColumn(); /*    ZAT_AMT */ im::TextUnformatted(std::format("{:13.2f}", inc.  ZAT_AMT).c_str());
							im::TableNextColumn(); /*   FAKT_AMT */ im::TextUnformatted(std::format("{:13.2f}", inc. FAKT_AMT).c_str());
							im::TableNextColumn(); /*  PLANS_AMT */ im::TextUnformatted(std::format("{:13.2f}", inc.PLANS_AMT).c_str());
						}

						im::EndTable();
					}

					im::EndTabItem();
				}
			}

			im::EndTabBar();
		}

		im::End();
	}
}


// Stash //

class pricetag { // stores not only the acutal `price` value, but also `tag` - stringed simplified view of the 'price'
	money_t price{}; // numeric 'money value'   (like: $5 252)
	string    tag{}; // textual simplified view (like: $5K or $5,2K or $5,3K or $5,25K)

public:
	pricetag(const money_t &price) : price(price) { UpdateTag(); }

	pricetag& operator=(      pricetag& v) { std::swap(price, v.price); UpdateTag(); return *this; }
	pricetag  operator+(const pricetag& v) { return pricetag(price + v.price); }

	operator money_t() const { return price; }

	friend std::ostream& operator<<(std::ostream &os, const pricetag& v) { os << v.tag; return os;}

	void    SetPrice(const string &s) { price = std::stold(s); UpdateTag(); }
	money_t GetPrice   () const { return price; }
	string  GetPriceStr() const { return std::to_string(price); }
	string  GetTag     () const { return tag  ; }

	static string RuleTag(const money_t &money)              { return std::to_string(money); } //1 action placeholder
	static void   RuleTag(const money_t &money, string &tag) { tag =  std::to_string(money); } //1 action placeholder
	//void UpdateTag() { tag = RuleTag(price); }
	void UpdateTag() { RuleTag(price, tag); }
};


} // namespace test


// Stash //

//// convert _IncCodeName_: 'map' -> 'vector'
//static_assert(std::is_same_v<decltype(codeNameMap)::value_type, std::pair<const uint32_t, string>>);
//std::vector<std::pair<uint32_t, string>> codeNameVec(std::make_move_iterator(codeNameMap.begin()), std::make_move_iterator(codeNameMap.end()));
//codeNameMap.clear();

/* for (const auto &colname : "ZAT_AMT", "PLANS_AMT", "FAKT_AMT")
	if (auto s = csv.GetCell<string>(colname, i); s.empty())
		std::cout << "TRACE: empty money-val string, line: " << i + 2 << ", colname: " << colname << "\n"; */
