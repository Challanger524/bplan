#pragma once

#include "test/tests.hpp"
#include "test/csv/CsvTable.hpp"

#include <imgui.h>

#include <memory>
#include <array>

namespace bplan {

inline bool MenuItem(const char *label, bool *p_selected = nullptr) { return im::MenuItem(label, nullptr, p_selected); } // ImGui wrapper

} // namespace bplan

class App;

class Test final {
public:
	enum testsE : size_t { CSV_TABLE, TESTS_COUNT_ };

private:
	App &app [[maybe_unused]];
	std::array<std::unique_ptr<test::ITests>, TESTS_COUNT_> tests{}; // array of pointers to the tests

public:
	Test(App *app) : app{*app} {}

	void Menu() {
		if (im::BeginMenu("Test")) {
			im::SeparatorText(".csv");
			im::Indent(); /* scope: syntax */ {
				im::SeparatorText("one-run");
				if (im::MenuItem("console: income Chernihiv 23 quarter"))           { test::ReadCsvQ(); }
				if (im::MenuItem("console: income Chernihiv 23 month (filter: T)")) { test::CsvFilterT(); }
				im::SeparatorText("loop");
				if (im::MenuItem("imgui  : income Chernihiv 23 month (table)"))     { this->Switch(CSV_TABLE); }
				im::Unindent();
			}
			im::SeparatorText("demo");
			im::EndMenu();
		}
	}

	void operator()() {
		for (const auto &t : tests) if (t) t->operator()();
	}

	void Enable(const testsE test) { this->Switch(test); }
	void Switch(const testsE test)
	{
		if (this->tests[test]) this->tests[test].release();
		else {
			switch (test) {
				case CSV_TABLE: this->tests[test] = std::make_unique<test::CsvTable>(); break;

				default: assert(false); break;
			}
		}
	}
};


// Stash //


//#define BindThis(method) [this]() { this->method(); } // Bind pointer `this` to the _class::_`method`
//std::array<     bool            , TESTS_COUNT_> showtestB{}; // booleans
//std::array<std::function<void()>, TESTS_COUNT_> showtestF{ BindThis(CsvTable) }; // bind(&Test::CsvTable, this) // [this](){this->CsvTable();}
