#pragma once

#include "test/tests.hpp"

#include <rapidcsv.h>
#include <imgui.h>

#include <type_traits>
#include <functional>
//#include <memory>
#include <array>

#define BindThis(method) [this]() { this->method(); } // Bind pointer `this` to the _class::_`method`

namespace bplan { inline bool MenuItem(const char *label, bool *p_selected) { return im::MenuItem(label, nullptr, p_selected); } }

class App;

class Test {
public:
	enum testsE : size_t { CSV_TABLE, TESTS_COUNT_ };

private:
	App &app;
	rapidcsv::Document csv{};

	std::array<     bool            , TESTS_COUNT_> showtestB{}; // booleans
	std::array<std::function<void()>, TESTS_COUNT_> showtestF{ BindThis(CsvTable) }; // bind(&Test::CsvTable, this) // [this](){this->CsvTable();}

public:
	Test(App *app) : app(*app) {}

	void Menu() {
		if (im::BeginMenu("Test")) {
			im::SeparatorText("oneruns");
			im::Indent();
			{
				im::SeparatorText(".csv");
				if (im::MenuItem("console: income Chernihiv 23 quarter"))           { test::ReadCsvQ(); }
				if (im::MenuItem("console: income Chernihiv 23 month (filter: T)")) { test::CsvFilterT(); }
				if (bp::MenuItem("imgui  : income Chernihiv 23 month (table)", &showtestB[CSV_TABLE])) { if (showtestB[CSV_TABLE]) csv.Clear(); }
				im::Unindent();
			}
			im::SeparatorText("demo");
			im::EndMenu();
		}
	}

	void operator()() {
		for (size_t i = 0; i < showtestB.size(); i++)
			if (this->showtestB[i])
				this->showtestF[i]();
	}

	void Enable(testsE test) { csv.Clear(); showtestB[test] = true; }

private:
	void CsvTable();
};
