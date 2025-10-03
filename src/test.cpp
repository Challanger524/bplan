#include "test.hpp"
#include "test/csv/CsvTable.hpp"
#include "test/csv/CsvGet.hpp"
#include "test/sql/sqlite_orm.hpp"
#include "test/sql/sql_server/odbc_wrap.hpp"
#include "test/sql/sql_server/odbc_nano.hpp"

#include <imgui.h>

#include <array>
#include <print>
#include <memory>
#include <exception>


namespace bplan {

inline bool MenuItem(const char *label, bool selected = false) { return im::MenuItem(label, nullptr, selected); } // ImGui wrapper

} // namespace bplan

Test::Test() {}

void Test::Menu() {
	if (im::BeginMenu("Test")) {
		im::SeparatorText(".csv");
		im::Indent(); /* scope: syntax */ {
			im::SeparatorText("one-run");
			if (im::MenuItem("console: income Chernihiv 23 quarter"))           { test::ReadCsvQ(); }
			if (im::MenuItem("console: income Chernihiv 23 month (filter: T)")) { test::CsvFilterT(); }
			if (im::MenuItem("console: income Chernihiv 23 quarter (convert)")) { test::ConvertCsv(); }
			im::SeparatorText("loop");
			this->MenuItem("imgui   : income Chernihiv 23 month (table)", CSV_TABLE    );
			this->MenuItem("download: selected budget to show it"       , CSV_GET      );
			this->MenuItem("sql     : SQLite ORM"                       , SQL_L_ORM    );
			this->MenuItem("sql     : SQL Server 2019: cpp-odbc-wrapper", SQL_ODBC_WRAP);
			im::Unindent();
		}

		//im::SeparatorText("demo");
		im::EndMenu();
	}
}

void Test::operator()() {
	for (const auto &t : tests) if (t) t->operator()();
}

void Test::MenuItem(const char *label, Test::testsE test) {
	if (bp::MenuItem(label, this->Enabled(test))) { this->Switch(test); }
}

bool Test::Enabled(const testsE test) const { return this->tests[test].operator bool(); }
void Test::Enable(const testsE test) { this->Switch(test); }
void Test::Switch(const testsE test)
{
	if (this->tests[test]) this->tests[test].release();
	else { // construct the test
		try {
			switch (test) {
				case CSV_TABLE: this->tests[test] = std::make_unique<test::CsvTable>(); break;
				case CSV_GET  : this->tests[test] = std::make_unique<test::CsvGet  >(); break;

			#ifndef TEST_ODR_VIOLATION
				case SQL_L_ORM: this->tests[test] = std::make_unique<test::sqlite::SqliteOrm>(); break;
			#else
				case SQL_L_ORM: this->tests[test] = std::make_unique<test::SqliteOrm>(); break;
			#endif

				case SQL_ODBC_WRAP : this->tests[test] = std::make_unique<test::odbc_wrap::SqlServer>(); break;
				case SQL_ODBC_NANO : this->tests[test] = std::make_unique<test::odbc_nano::SqlServer>(); break;

				default: assert(false); break;
			}

		} catch (std::runtime_error &e) { std::println(stderr, "Fail to construct test: {}()", e.what()); }
	}
}
