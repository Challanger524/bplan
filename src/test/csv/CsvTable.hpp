#ifndef TEST_CSV_CSV_TABLE_HPP_
#define TEST_CSV_CSV_TABLE_HPP_
#else  //  include anti-guard
#error "re-include of non-library header"
#endif //  https://stackoverflow.com/a/32628153/11753532

#include "test/tests.hpp"
#include "UA/budget/incomes.hpp"

#include <rapidcsv.h>

#include <array>

namespace test {

class CsvTable : public ITests
{
	rapidcsv::Document csv{};
	std::array<size_t, UA::budget::incomes::LABEL_COUNT_> labelI{}; // I - id positions (real)

public:
	virtual
	~CsvTable() override = default;
	 CsvTable();

	virtual void operator()() override;
};

} // namespace test


// Stash //

// declaring each variable separately, overcomlicated design
//size_t REP_PERIOD = -1, COD_INCO = -1, NAME_INC = -1, ZAT_AMT = -1, PLANS_AMT = -1, FAKT_AMT = -1;
