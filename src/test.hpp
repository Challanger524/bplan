#pragma once

#include "test/tests.hpp"

#include <imgui.h>

#include <memory>
#include <array>

class Test final {
public:
	enum testsE : size_t { CSV_TABLE, CSV_GET, SQL_L_ORM, TESTS_COUNT_ };

private:
	std::array<std::unique_ptr<test::ITests>, TESTS_COUNT_> tests{}; // array of pointers to the tests

public:
	Test();

	void Menu();

	void operator()();

	bool Enabled(testsE test) const;
	void Enable(testsE test);
	void Switch(testsE test);
};


// Stash //


//#define BindThis(method) [this]() { this->method(); } // Bind pointer `this` to the _class::_`method`
//std::array<     bool            , TESTS_COUNT_> showtestB{}; // booleans
//std::array<std::function<void()>, TESTS_COUNT_> showtestF{ BindThis(CsvTable) }; // bind(&Test::CsvTable, this) // [this](){this->CsvTable();}
