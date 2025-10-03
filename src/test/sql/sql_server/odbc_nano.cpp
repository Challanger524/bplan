#include "test/sql/sql_server/odbc_nano.hpp"

#include "UA/budget/local/incomes.hpp"
#include "bplan/chrono.hpp"

#include <nanodbc/nanodbc.h>
#include <nanodbc/example/example_unicode_utils.h> // `convert()` for nanodbc tests

#include <rapidcsv.h>
#include <imgui.h>

#include <map>
#include <set>
#include <print>
#include <format>
#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <exception>
#include <type_traits>
#include <string.h>


namespace test::odbc_nano { // "nanodbc"

namespace rcv = rapidcsv;
namespace ndbc = nanodbc;
using std::string;

static constexpr auto CSV_PATH = "res/testing/local_2555900000_INCOMES_2023_month.csv";

// for nanodbc tests, copied from 'nanodbc/example/usage.cpp'
void show(nanodbc::result& results);

SqlServer::SqlServer()
{
	try {

		{ // print available SQL drivers
			const std::list<ndbc::driver> drivers = ndbc::list_drivers();
			std::println("SQL drivers list:");
			for (const auto &driver : drivers) {
				std::println("  {}", driver.name);
				//for (const auto &attr : driver.attributes) std::println("    {:-<16} {}", attr.keyword, attr.value);
			}
			std::println("");
		}

		constexpr long timeout = 2; // seconds
		this->con = ndbc::connection("Driver={ODBC Driver 17 for SQL Server};server=localhost;database=DemoDB;trusted_connection=Yes;", timeout);
		std::cout << "Connected with driver: " << convert(con.driver_name()) << std::endl;

	#if 0 // example
		nanodbc::execute(con, NANODBC_TEXT("DROP   TABLE IF EXISTS simple_test;"));
		nanodbc::execute(con, NANODBC_TEXT("create table simple_test (a int, b varchar(10));"));

		{ // Direct execution
			nanodbc::execute(con, NANODBC_TEXT("insert into simple_test values (1, 'one');"));
			nanodbc::execute(con, NANODBC_TEXT("insert into simple_test values (2, 'two');"));
			nanodbc::execute(con, NANODBC_TEXT("insert into simple_test values (3, 'tri');"));
			nanodbc::execute(con, NANODBC_TEXT("insert into simple_test (b) values ('z');"));

			nanodbc::result results = execute(con, NANODBC_TEXT("select * from simple_test;"));
			show(results);
		}

		{ // Accessing results by name, or column number
			nanodbc::result results = execute(con, NANODBC_TEXT("select a as first, b as second from simple_test where a = 1;"));
			results.next();
			auto const value = results.get<nanodbc::string>(1);
			std::cout << std::endl << results.get<int>(NANODBC_TEXT("first")) << ", " << convert(value) << std::endl;
		}

		{ // Binding parameters
			nanodbc::statement statement(con);

			// Inserting values
			prepare(statement, NANODBC_TEXT("insert into simple_test (a, b) values (?, ?);"));
			const int eight_int = 8;
			statement.bind(0, &eight_int);
			nanodbc::string const eight_str = NANODBC_TEXT("eight");
			statement.bind(1, eight_str.c_str());
			execute(statement);

			// Inserting null values
			prepare(statement, NANODBC_TEXT("insert into simple_test (a, b) values (?, ?);"));
			statement.bind_null(0);
			statement.bind_null(1);
			execute(statement);

			// Inserting multiple null values
			prepare(statement, NANODBC_TEXT("insert into simple_test (a, b) values (?, ?);"));
			statement.bind_null(0, 2);
			statement.bind_null(1, 2);
			execute(statement, 2);

			prepare(statement, NANODBC_TEXT("select * from simple_test;"));
			nanodbc::result results = execute(statement);
			show(results);
		}
	#endif

		ndbc::execute(con, NANODBC_TEXT("DROP   TABLE IF EXISTS INCOMES"));
		ndbc::execute(con, NANODBC_TEXT("DROP   TABLE IF EXISTS IncCodeNames"));

		ndbc::execute(con, NANODBC_TEXT("CREATE TABLE IncCodeNames (id int NOT NULL IDENTITY(0,1) PRIMARY KEY, COD_INCO int UNIQUE, NAME_INC varchar(max))"));
		ndbc::execute(con, NANODBC_TEXT("CREATE TABLE INCOMES      (COD_BUDGET varchar(11),"
		                                                           "REP_PERIOD date       ,"
		                                                           "COD_INCO   int        ,"
		                                                           "NAME_INC   int FOREIGN KEY REFERENCES IncCodeNames(id),"
		                                                           "ZAT_AMT    float      ,"
		                                                           "PLANS_AMT  float      ,"
		                                                           "FAKT_AMT   float      )"
		                               ));

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
					const string   nameInc =            csv.GetCell<string>("NAME_INC", i);
					if (!codeNameMap.contains(codeInc)) codeNameMap[codeInc] = nameInc;
				}
				catch(std::exception &e) { std::cerr << e.what() << '\n'; }
			}

			[[maybe_unused]] int idc = 0; // id counter
			for (auto &cn : codeNameMap) {
				ndbc::statement statement(con);
				ndbc::prepare  (statement, NANODBC_TEXT("INSERT INTO [IncCodeNames] VALUES (?, ?)"));
				const       int     incCode     = scast<int>(cn.first);
				const ndbc::string &incCodeName = cn.second;
				statement.bind(0,  &incCode);
				statement.bind(1,   incCodeName.c_str());
				ndbc::execute(statement);
			}
		}

		// Fill `Income` DB Table and connect unique Foreign Key refs to Code Names
		{
			const auto rowCount = csv.GetRowCount();
			for (size_t i = 0; i < rowCount; i++)
			{
				if (csv.GetCell<string>("FUND_TYP", i) != "T") continue; // skip non-"together" entries

				Income income;

				income.COD_BUDGET = csv.GetCell<string>("COD_BUDGET", i);

				const string date = csv.GetCell<string>("REP_PERIOD", i); // like: 01.2025
				bp::chrono::from_stream(date, "%m.%Y", income.REP_PERIOD);

				income.COD_INCO = csv.GetCell<uint32_t>("COD_INCO", i);

				ndbc::statement stSelect(con);
				ndbc::prepare  (stSelect, NANODBC_TEXT("SELECT [id] FROM [IncCodeNames] WHERE [COD_INCO] = ?"));
				const int          incCode = scast<int>(income.COD_INCO);
				stSelect.bind(0, &incCode);
				ndbc::result res = ndbc::execute(stSelect);
				//res.columns();
				for (short i = 1; res.next(); i++) {
					if (i > 1) std::cerr << "WARN: res->next() count == '" << i << "', expected 1\n";
					else income.NAME_INC = res.get<int>(0);
				}

				// wrapper to intercept empty strings and return `auto(0)` in such case
				auto StoWrapper = [](auto(*std_sto)(const string&, size_t*), const string &str) -> std::invoke_result_t<decltype(std_sto), const string&, size_t*> {
					if (str.empty()) return 0; // cast to the determined return type
					else             return std_sto(str, nullptr); //~ catch
				};

				income.  ZAT_AMT = StoWrapper(std::stold, csv.GetCell<string>(  "ZAT_AMT", i));
				income.PLANS_AMT = StoWrapper(std::stold, csv.GetCell<string>("PLANS_AMT", i));
				income. FAKT_AMT = StoWrapper(std::stold, csv.GetCell<string>( "FAKT_AMT", i));

				ndbc::statement stInsert(con);
				ndbc::prepare(  stInsert, NANODBC_TEXT("INSERT INTO [INCOMES] VALUES (?, ?, ?, ?, ?, ?, ?)"));
				//const       int  incCode = scast<int>(income.COD_INCO);
				const ndbc::date incDate{.year  = scast<int16_t>(bp::chrono::get(income.REP_PERIOD.year ())),
				                         .month = scast<int16_t>(bp::chrono::get(income.REP_PERIOD.month())),
				                         .day = 1};
				stInsert.bind(0,  income.COD_BUDGET.c_str());
				stInsert.bind(1, &incDate                  );
				stInsert.bind(2, &incCode                  );
				stInsert.bind(3, &income.NAME_INC          );
				stInsert.bind(4, &income.  ZAT_AMT         );
				stInsert.bind(5, &income.PLANS_AMT         );
				stInsert.bind(6, &income. FAKT_AMT         );
				ndbc::execute(stInsert);
			}

		}

		// Prepare member data
		this->codeNames.clear();
		this->codeNames.reserve(80); //!
		this->codeNames.shrink_to_fit();
		{
			ndbc::statement stSelect(con);
			ndbc::prepare  (stSelect, NANODBC_TEXT("SELECT * FROM [IncCodeNames]"));
			ndbc::result res = ndbc::execute(stSelect);
			assert(res.columns() == 3);
			while (res.next()) { this->codeNames.push_back(IncCodeName{res.get<int>(0), res.get<int>(1), std::move(res.get<std::string>(2))}); }
		}

		// get months
		this->months.clear();
		this->months.reserve(12);
		this->months.shrink_to_fit();
		{
			std::set<uint> monthSet;
			ndbc::result res = ndbc::execute(con, NANODBC_TEXT("SELECT DISTINCT [REP_PERIOD] FROM [INCOMES] ORDER BY [REP_PERIOD] ASC"));

			while (res.next()) {
				auto dat = res.get<ndbc::date>(0);
				if (monthSet.insert(scast<uint>(dat.month)).second == false) std::println(stderr, "ERR: Odd date: {}.{}.{}", dat.year, dat.month, dat.day);
			}

			this->months = decltype(this->months)(monthSet.begin(), monthSet.end()); // copy set -> vector
		}
		//std::ranges::sort(this->months);
		//std::println("{}", this->months);

		// print Code-Name table
		//for (const auto &cn : this->codeNames) std::println("{} | {} | {}", cn.id, cn.COD_INCO, cn.NAME_INC);

#if 1 // print few INCOMES entires from DB
		{
			auto ToStrYM = [](const ndbc::date &d) { return std::format("{}.{}", d.year, d.month); };
			ndbc::result res = ndbc::execute(con, NANODBC_TEXT("SELECT TOP (12) * FROM [INCOMES]"));
			std::cout << "\n";
			while (res.next()) { std::cout << res.get<nanodbc::string>(0) << " " << ToStrYM(res.get<ndbc::date>(1)) << " " << res.get<int>(2) << " " << res.get<int>(3) << " " << res.get<double>(4) << " " << res.get<double>(5) << " " << res.get<double>(6) << std::endl; }
		}
#endif

	} catch (const std::exception& e) { std::cerr << e.what() << std::endl; throw std::runtime_error(std::format("odbc_nano::{}", __func__)); }
}

void SqlServer::operator()()
{
	constexpr ImGuiWindowFlags wFlags =
	ImGuiWindowFlags_NoCollapse            |
	ImGuiWindowFlags_NoSavedSettings       |
	ImGuiWindowFlags_AlwaysAutoResize      |
	0;

	im::SetNextWindowSizeConstraints(ImVec2{0, 0}, ImVec2{FLT_MAX, 400});

	if (im::Begin("TEST_SQL_SQL_SERVER", nullptr, wFlags))
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
					if (this->incomes.empty())
					{
						try {
							ndbc::statement stSelect(con);
							ndbc::prepare  (stSelect, NANODBC_TEXT("SELECT * FROM [INCOMES] WHERE MONTH(REP_PERIOD) = ? ORDER BY [ZAT_AMT] ASC, [FAKT_AMT] DESC"));
							//const int         mon = scast<int>(month);
							stSelect.bind(0, &month); //? uint vs int
							ndbc::result res = ndbc::execute(stSelect);

							while (res.next()) {
								auto codBud = res.get<ndbc::string>(0);
								auto ndDate = res.get<ndbc::date  >(1); // nanodbc-Date
								this->incomes.push_back(Income{
									.COD_BUDGET = std::move(codBud),
									.REP_PERIOD = std::chrono::year_month{std::chrono::year{ndDate.year}, std::chrono::month{scast<uint>(ndDate.month)}},
									.COD_INCO   = scast<uint>(res.get<int>(2)),
									.NAME_INC   = res.get<int   >(3),
									.ZAT_AMT    = res.get<double>(4),
									.PLANS_AMT  = res.get<double>(5),
									.FAKT_AMT   = res.get<double>(6)
								});
							}

						} catch (const std::exception& e) { std::cerr << e.what() << std::endl; }

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
									                             [&id = inc.NAME_INC](const IncCodeName &cn) { return cn.id == id; });
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

void show(nanodbc::result& results) // _unformatted_
{
    const short columns = results.columns();
    long rows_displayed = 0;

    std::cout << "\nDisplaying " << results.affected_rows() << " rows "
         << "(" << results.rowset_size() << " fetched at a time):" << std::endl;

    // show the column names
    std::cout << "row\t";
    for (short i = 0; i < columns; ++i)
        std::cout << convert(results.column_name(i)) << "\t";
    std::cout << std::endl;

    // show the column data for each row
    nanodbc::string const null_value = NANODBC_TEXT("null");
    while (results.next())
    {
        std::cout << rows_displayed++ << "\t";
        for (short col = 0; col < columns; ++col)
        {
            auto const value = results.get<nanodbc::string>(col, null_value);
            std::cout << "(" << convert(value) << ")\t";
        }
        std::cout << std::endl;
    }
}


} // namespace test::odbc_wrap


// Stash //
