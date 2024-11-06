#include "test/tests.hpp"

//#define HAS_CODECVT
#include <rapidcsv.h>

#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <array>
#include <assert.h>

namespace test
{

namespace rcv = rapidcsv;

constexpr auto FPATH_CSV_Q = "res/testing/local_2555900000_INCOMES_2023_quarter.csv";

constexpr std::array<size_t, 4> pickColumns = {3, 5, 7, 6}; // way to pick columns (and display in declared order)
constexpr      size_t     rows_per_quarter = 5;
constexpr std::streamsize colWidth = 13; // column width

void ReadCsvQ() // Q - quarter
{
	std::cout << "\nTesting: " << FPATH_CSV_Q << "\n";

	assert(std::filesystem::exists(FPATH_CSV_Q)); //? replace with runtime check
	rcv::Document csv(FPATH_CSV_Q, rcv::LabelParams(), rcv::SeparatorParams(';'));

	{ // List all lables
		const auto &labels = csv.GetColumnNames();
		std::cout << "Column labels(" << labels.size() << "):";
		for (const auto &label : labels) std::cout << " " << label;
		std::cout << "\n";
	}

	{ // Print table with few first elements per quater
		std::cout << "\nSome table content (up to '" << rows_per_quarter << "' entries per quarter):\n";

		// S0: print labels
		for (const auto col : pickColumns) {
			try { std::cout << std::setw(colWidth) << csv.GetColumnName(col) << ';'; }
			catch (std::exception &e) { std::cerr << "\nEXCEP: " << e.what() << "\n"; abort(); }
		}
		std::cout << '\n';

		// S1: print quarters
		size_t rows_count = 0;
		std::string period;
		for (size_t i = 0; i < csv.GetRowCount(); i++)
		{
			const auto &row = csv.GetRow<std::string>(i);

			if (row.size() < *std::max_element(pickColumns.begin(), pickColumns.end())) {
				std::cerr << "WARN: row has '" << row.size() << "' elements (<" << *std::max_element(pickColumns.begin(), pickColumns.end()) << ")\n";
				continue;
			};

			if (row.empty()  ) continue;
			if (row[1] != "T") continue; // Т (FUND_TYP) - разом
			if (period != row[0]) {
				rows_count = 0;
				period = row[0];
				std::cout << period << ":\n";
			}

			if (rows_count >= rows_per_quarter) continue;


			// S2: list (few) rows

			for (const auto col : pickColumns) {
				std::cout << std::setw(colWidth) << row[col] << ';';
			}
			std::cout << '\n';

			rows_count++;
		}
	}
}

} // test namespace
