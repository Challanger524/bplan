/* This test aims to utilize std::ranges(::views) usage */

#include "test/tests.hpp"
#include "UA/budget/local/incomes.hpp"

#include <rapidcsv.h>

#include <functional>
#include <exception>
#include <algorithm> // ranges(more)
#include <iterator>
//#include <iostream>
#include <utility>
#include <ranges>    // ranges, views
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <print>
#include <set>
//#include <assert.h>

namespace test {


namespace { // avoid ODR violation from same-named `struct Income` from ../sql/sqlite_orm.hpp. Very hard to trace and debug such implicit ODR stuff.

using std::string;

struct Income {
	string   REP_PERIOD{};
	char     FUND_TYP  {};
	string   COD_BUDGET{};
	uint32_t COD_INCO  {};
	//string   NAME_INC  {};
	//money_t    ZAT_AMT {};
	//money_t  PLANS_AMT {};
	money_t   FAKT_AMT {};

	void println() const { std::println("{} {} {} {:.2f}", REP_PERIOD, FUND_TYP, COD_INCO, FAKT_AMT); }
};

} // namespace

namespace    rcv = rapidcsv;
namespace ranges = std::ranges       ;
namespace  views = std::ranges::views;
//using std::string;

constexpr static auto FPATH_CSV_Q = "res/testing/local_2555900000_INCOMES_2023_quarter.csv";

template<ranges::input_range R> void printInc(R &&r) { ranges::for_each(r, [](const Income &i){ i.println(); }); }
//void printInc(auto &&r) { ranges::for_each(r, [](const Income &i) { i.println(); }); }
void printInc() { std::println(""); }

void ConvertCsv()
{
	std::println("Testing: {}", FPATH_CSV_Q);

	try {
		rcv::Document csv;
		csv.Load(FPATH_CSV_Q, rcv::LabelParams(), rcv::SeparatorParams(';'));

		std::vector<Income> incomes;
		incomes.reserve(csv.GetRowCount());

		for (size_t i = 0; i < csv.GetRowCount(); i++)
		{
			using namespace UA::budget::incomes;
			const auto &row = csv.GetRow<std::string>(i);

			if (row.empty()) continue;

			Income inc;
			/*                       */ inc.REP_PERIOD =            row[REP_PERIOD];
			if (!row[FUND_TYP].empty()) inc.FUND_TYP   =            row[FUND_TYP  ].front();
			/*                       */ inc.COD_BUDGET =            row[COD_BUDGET];
			if (!row[COD_INCO].empty()) inc.COD_INCO   = std::stoul(row[COD_INCO]);
			if (!row[FAKT_AMT].empty()) inc.FAKT_AMT   = std::stold(row[FAKT_AMT]);
			//if (!row[ZAT_AMT].empty()) inc.ZAT_AMT     = std::stold(row[ZAT_AMT]);

			incomes.emplace_back(std::move(inc));
		}

		// shuffle data
		std::mt19937 randgen{std::random_device()()};
		ranges::shuffle(incomes, randgen);
		printInc();
		std::println("ranges::shuffle(incomes, gen):");
		printInc(incomes | views::take(6)); printInc();

		// filter out FUND_TYP != T
		{
			//auto rmC = ranges::remove(incomes, 'C', &Income::FUND_TYP); incomes.erase(rmC.begin(), rmC.end());
			//auto rmS = ranges::remove(incomes, 'S', &Income::FUND_TYP); incomes.erase(rmS.begin(), rmS.end());
			auto rm = ranges::remove_if(incomes, [](char c) { return c != 'T'; }, &Income::FUND_TYP); // preserve only 'T'
			incomes.erase(rm.begin(), rm.end());
			std::println("ranges::remove_if(incomes, [](char c) {{ return c != 'T'; }}, &Income::FUND_TYP):");
			printInc(incomes | views::take(5)); printInc();
		}

		ranges::sort(incomes, ranges::less{}, &Income::REP_PERIOD);
		std::println("ranges::sort(incomes, ranges::less{{}}, &Income::REP_PERIOD):");
		printInc(incomes | views::take(3)); printInc();

		// process periods: get unique periods, get border iterators, sort them
		{
			std::set<string> periodUniques; // unique periods
			ranges::for_each(incomes, [&](const string &s) { periodUniques.insert(s); }, &Income::REP_PERIOD);
			std::println("std::set<string> periodUniques:");
			std::println("{}", periodUniques); printInc();

			std::vector<decltype(incomes)::iterator> periodIterats; // sane view on period chunks
			for (const auto &up : periodUniques)     periodIterats.push_back(ranges::find(incomes, up, &Income::REP_PERIOD));

			std::println("std::vector<decltype(incomes)::iterator> periodIterats:");
			for (const auto &it : periodIterats) it->println();
			printInc();

			// sort each period (chunk)
			for (size_t i = 0; i < periodIterats.size(); i++) {
				const auto &periodBegin = periodIterats[i];
				const auto &periodEnd   = i + 1 < periodIterats.size() ? periodIterats[i + 1] : incomes.end();

				ranges::sort(periodBegin, periodEnd, ranges::greater{}, &Income::COD_INCO);
			}

			std::println("ranges::sort(periodBegin, periodEnd, ranges::greater{{}}, &Income::COD_INCO):");
			printInc(incomes | views::take(5)); printInc();
		}

		// create sample of `sample_count` random entries
		constexpr size_t sample_count = 20;
		std::vector<Income> sample;
		/*               */ sample.reserve(sample_count);
		ranges::sample(incomes, std::back_inserter(sample), sample_count, randgen);

		std::println("ranges::sample(incomes, std::back_inserter(sample), {}, randgen):", sample_count);
		printInc(sample); //printInc();

	} catch(std::exception &e) { std::println(stderr, "EXCEP: {}", e.what()); }

}


} // namespace test
