#include "test.hpp"
#include "test/csv/CsvTable.hpp"
#include "test/csv/CsvGet.hpp"
#include "test/sql/sqlite_orm.hpp"

#include <imgui.h>

#include <memory>
#include <array>

namespace bplan {

inline bool MenuItem(const char *label, bool *p_selected = nullptr) { return im::MenuItem(label, nullptr, p_selected); } // ImGui wrapper

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
			if (im::MenuItem("imgui  : income Chernihiv 23 month (table)"))     { this->Switch(CSV_TABLE); }
			if (im::MenuItem("network: boost::beast download budget"     ))     { this->Switch(CSV_GET  ); }
			if (im::MenuItem("sql    : SQLite ORM"                       ))     { this->Switch(SQL_L_ORM); }
			im::Unindent();
		}

		im::SeparatorText("demo");
		im::EndMenu();
	}
}

void Test::operator()() {
	for (const auto &t : tests) if (t) t->operator()();
}

void Test::Enable(const testsE test) { this->Switch(test); }
void Test::Switch(const testsE test)
{
	if (this->tests[test]) this->tests[test].release();
	else {
		switch (test) {
			case CSV_TABLE: this->tests[test] = std::make_unique<test::CsvTable>(); break;
			case CSV_GET  : this->tests[test] = std::make_unique<test::CsvGet  >(); break;
			case SQL_L_ORM: this->tests[test] = std::make_unique<test::SqliteOrm>(); break;

			default: assert(false); break;
		}
	}
}
