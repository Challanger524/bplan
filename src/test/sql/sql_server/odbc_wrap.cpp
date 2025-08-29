#include "test/sql/sql_server/odbc_wrap.hpp"

#include "UA/budget/local/incomes.hpp"
#include "bplan/chrono.hpp"

#include <odbc/Environment.h>
#include <odbc/Connection.h>
#include <odbc/Exception.h>
#include <odbc/PreparedStatement.h>
#include <odbc/ResultSet.h>

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


namespace test::odbc_wrap { // "odbc-cpp-wrapper"

namespace rcv = rapidcsv;
using std::string;

static constexpr auto CSV_PATH = "res/testing/local_2555900000_INCOMES_2023_month.csv";

SqlServer::SqlServer()
{
	try {
		this->env = odbc::Environment::create();
		this->con = env->createConnection();

		std::vector<odbc::DriverInformation> driverInfos = this->env->getDrivers();
		std::println("SQL drivers list:");
		for (const auto &di : driverInfos) {
			std::println("  {}", di.description);
			//for (const auto &attr : di.attributes) std::println("    {:-<16} {}", attr.name, attr.value);
		}
		std::println("");

		constexpr auto SQL_DRIVER_   = "ODBC Driver 17 for SQL Server";
		constexpr auto SQL_SERVER_   = "localhost";
		constexpr auto SQL_DATABASE_ = "DemoDB";

		const auto connStr = std::format("Driver={{{}}};server={};database={};trusted_connection=Yes;", SQL_DRIVER_, SQL_SERVER_, SQL_DATABASE_);
		std::println("Trying to connect with: {}", connStr);
		std::println("");

		con->setConnectionTimeout(1);
		con->setLoginTimeout     (2);
		//con->connect("Driver={ODBC Driver 17 for SQL Server};server=localhost;database=DemoDB;trusted_connection=Yes;");
		con->connect(connStr.c_str());
		con->setAutoCommit(false);

	#if 0 // example (refined)
		con->connect("Driver={ODBC Driver 17 for SQL Server};server=localhost;database=DemoDB;trusted_connection=Yes;");

		//odbc::PreparedStatementRef psDrop   = con->prepareStatement("DROP   TABLE IF EXISTS TAB")                  ; psDrop  ->executeQuery();
		//odbc::PreparedStatementRef psCreate = con->prepareStatement("CREATE TABLE TAB (ID int, DATA varchar(255))"); psCreate->executeQuery();
		con->prepareStatement("DROP   TABLE IF EXISTS TAB"                  )->executeQuery();
		con->prepareStatement("CREATE TABLE TAB (ID int, DATA varchar(255))")->executeQuery();
		odbc::PreparedStatementRef psInsert = con->prepareStatement("INSERT INTO  TAB (ID, DATA) VALUES (?, ?)");
		psInsert->setInt(1, 101); psInsert->setCString(2, "One hundred one"); psInsert->addBatch();
		psInsert->setInt(1, 102); psInsert->setCString(2, "One hundred two"); psInsert->addBatch();
		psInsert->executeBatch();

		con->commit();

		//odbc::ResultSetRef rs = con->prepareStatement("SELECT ID, DATA FROM TAB WHERE ID > 100")->executeQuery();
		odbc::PreparedStatementRef psSelect = con->prepareStatement("SELECT ID, DATA FROM TAB WHERE ID > ?");
		psSelect->setInt(1, 100);
		odbc::ResultSetRef rs = psSelect->executeQuery();
		while (rs->next()) { std::cout << rs->getInt(1) << ", " << rs->getString(2) << std::endl; }
	#endif

		//con->commit();
		//con->prepareStatement("ALTER DATABASE DemoDB COLLATE Latin1_General_100_CI_AI_SC_UTF8; ")->executeQuery();
		//con->commit();

		con->prepareStatement("DROP   TABLE IF EXISTS INCOMES"     )->executeQuery();
		con->prepareStatement("DROP   TABLE IF EXISTS IncCodeNames")->executeQuery();

		//con->prepareStatement("CREATE TABLE IncCodeNames (id int NOT NULL IDENTITY(0,1) PRIMARY KEY, COD_INCO int UNIQUE, NAME_INC varchar(max) COLLATE LATIN1_GENERAL_100_CI_AS_SC_UTF8  )")->executeQuery();
		//con->prepareStatement("CREATE TABLE IncCodeNames (id int NOT NULL IDENTITY(0,1) PRIMARY KEY, COD_INCO int UNIQUE, NAME_INC varchar(max) COLLATE Latin1_General_100_CI_AS_KS_WS_SC_UTF8 )")->executeQuery();
		con->prepareStatement("CREATE TABLE IncCodeNames (id int NOT NULL IDENTITY(0,1) PRIMARY KEY, COD_INCO int UNIQUE, NAME_INC varchar(max))")->executeQuery();
		con->prepareStatement("CREATE TABLE INCOMES      (COD_BUDGET varchar(11),"
		                                                 "REP_PERIOD date       ,"
		                                                 "COD_INCO   int        ,"
		                                                 "NAME_INC   int FOREIGN KEY REFERENCES IncCodeNames(id),"
		                                                 "ZAT_AMT    float      ,"
		                                                 "PLANS_AMT  float      ,"
		                                                 "FAKT_AMT   float      )"
		                     )->executeQuery();

		con->commit();

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
				//storage.insert (IncCodeName{.id =    -1, .COD_INCO = cn.first, .NAME_INC = std::move(cn.second)}); // indexed from: 1
				//storage.replace(IncCodeName{.id = idc++, .COD_INCO = cn.first, .NAME_INC = std::move(cn.second)}); // indexed from: 0
				odbc::PreparedStatementRef psInsert = con->prepareStatement("INSERT INTO [IncCodeNames] VALUES (?, ?)");
				psInsert->setInt   (1, cn.first );
				psInsert->setString(2, cn.second);
				psInsert->executeQuery();
			}

			con->commit();
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

				odbc::PreparedStatementRef psSelect = con->prepareStatement("SELECT [id] FROM [IncCodeNames] WHERE [COD_INCO] = ?");
				psSelect->setInt(1, income.COD_INCO);
				odbc::ResultSetRef res = psSelect->executeQuery();
				for (short i = 1; res->next(); i++) {
					if (i > 1) std::cerr << "WARN: res->next() count == '" << i << "', expected 1\n";
					else income.NAME_INC = *res->getInt(1);
				}

				// wrapper to intercept empty strings and return `auto(0)` in such case
				auto StoWrapper = [](auto(*std_sto)(const string&, size_t*), const string &str) -> std::invoke_result_t<decltype(std_sto), const string&, size_t*> {
					if (str.empty()) return 0; // cast to the determined return type
					else             return std_sto(str, nullptr); //~ catch
				};

				income.  ZAT_AMT = StoWrapper(std::stold, csv.GetCell<string>(  "ZAT_AMT", i));
				income.PLANS_AMT = StoWrapper(std::stold, csv.GetCell<string>("PLANS_AMT", i));
				income. FAKT_AMT = StoWrapper(std::stold, csv.GetCell<string>( "FAKT_AMT", i));

				odbc::PreparedStatementRef psInsert = con->prepareStatement("INSERT INTO [INCOMES] VALUES (?, ?, ?, ?, ?, ?, ?)");
				psInsert->setString(1, income.COD_BUDGET);
				psInsert->setDate  (2, odbc::date(bp::chrono::get(income.REP_PERIOD.year()), bp::chrono::get(income.REP_PERIOD.month()), 1));
				psInsert->setInt   (3, income.  COD_INCO);
				psInsert->setInt   (4, income. NAME_INC );
				psInsert->setDouble(5, income.   ZAT_AMT);
				psInsert->setDouble(6, income. PLANS_AMT);
				psInsert->setDouble(7, income.  FAKT_AMT);
				psInsert->executeQuery();
			}

			con->commit();
		}

		// Prepare member data
		this->codeNames.clear();
		this->codeNames.reserve(80); //!
		this->codeNames.shrink_to_fit();
		{
			odbc::ResultSetRef res = con->prepareStatement("SELECT * FROM [IncCodeNames]")->executeQuery();
			while (res->next()) { this->codeNames.push_back(IncCodeName{*res->getInt(1), *res->getInt(2), std::move(*res->getString(3))}); }
		}

		// get months
		this->months.clear();
		this->months.reserve(12);
		this->months.shrink_to_fit();
		{
			std::set<uint> monthSet;
			odbc::ResultSetRef res = con->prepareStatement("SELECT DISTINCT [REP_PERIOD] FROM [INCOMES] ORDER BY [REP_PERIOD] ASC")->executeQuery();
			//while (res->next()) { this->months.push_back(scast<uint>(res->getDate(1)->month())); }
			while (res->next()) {
				auto dat = *res->getDate(1);
				if (monthSet.insert(scast<uint>(dat.month())).second == false) std::cerr << "ERR: Odd date: " << dat.toString() << "\n";
			}
			this->months = decltype(this->months)(monthSet.begin(), monthSet.end());
		}
		//std::ranges::sort(this->months);
		//std::println("{}", this->months);

		// print Code-Name table
		//for (const auto &cn : this->codeNames) std::println("{} | {} | {}", cn.id, cn.COD_INCO, cn.NAME_INC);

#if 0 // print few INCOMES entires from DB
		{
			odbc::ResultSetRef res = con->prepareStatement("SELECT TOP (12) * FROM [INCOMES]")->executeQuery();
			while (res->next()) { std::cout << *res->getString(1) << " " << res->getDate(2)->toString() << " " << *res->getInt(3) << " " << *res->getInt(4) << " " << *res->getDouble(5) << " " << *res->getDouble(6) << " " << *res->getDouble(7) << std::endl; }
		}
#endif

	} catch (const odbc::Exception& e) { std::cerr << e.what() << std::endl; throw std::runtime_error(std::format("odbc_wrap::{}", __func__)); }
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
							odbc::PreparedStatementRef stat = con->prepareStatement("SELECT * FROM [INCOMES] WHERE MONTH(REP_PERIOD) = ? ORDER BY [ZAT_AMT] ASC, [FAKT_AMT] DESC");
							stat->setInt(1, scast<int>(month));
							odbc::ResultSetRef res = stat->executeQuery();
							while (res->next()) {
								auto&      codBud = *res->getString(1);
								odbc::date& oDate = *res->getDate  (2);
								this->incomes.push_back(Income{
									.COD_BUDGET = std::move(codBud),
									.REP_PERIOD = std::chrono::year_month{std::chrono::year{oDate.year()}, std::chrono::month{scast<uint>(oDate.month())}},
									.COD_INCO   = scast<uint>(*res->getInt(3)),
									.NAME_INC   = *res->getInt   (4),
									.ZAT_AMT    = *res->getDouble(5),
									.PLANS_AMT  = *res->getDouble(6),
									.FAKT_AMT   = *res->getDouble(7)
								});
							}
						} catch (const odbc::Exception& e) { std::cerr << e.what() << std::endl; }

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


} // namespace test::odbc_wrap


// Stash //
