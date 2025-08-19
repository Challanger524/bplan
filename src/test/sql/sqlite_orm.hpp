#ifndef TEST_SQL_SQLITE_ORM_HPP_
#define TEST_SQL_SQLITE_ORM_HPP_
#else  //  include anti-guard
#error "re-include of non-library header"
#endif //  https://stackoverflow.com/a/32628153/11753532

#include "test/tests.hpp"

#include <memory>
#include <chrono>
#include <vector>
#include <string>

namespace test {

#ifndef TEST_ODR_VIOLATION
namespace sqlite { // avoid ODR violation from same-named `struct Income` from ../sql/sqlite_orm.hpp. Very hard to trace and debug such implicit ODR stuff.
#endif

struct IncCodeName {
	      int         id{-1};
	     uint32_t COD_INCO{};
	std::string   NAME_INC{}; // text translation
};

struct Income {
	std::        string     COD_BUDGET{};
	std::chrono::year_month REP_PERIOD{};
	uint32_t COD_INCO{};
	std::unique_ptr<int> NAME_INC{}; // foreign key (int)
	money_t   ZAT_AMT{};
	money_t PLANS_AMT{};
	money_t  FAKT_AMT{};

public:
	int  GetYear () const { return REP_PERIOD. year().operator          int(); }
	uint GetMonth() const { return REP_PERIOD.month().operator unsigned int(); }
	void SetYear ( int v) { REP_PERIOD = std::chrono::year_month(std::chrono::year(v), REP_PERIOD  .month( )); }
	void SetMonth(uint v) { REP_PERIOD = std::chrono::year_month(REP_PERIOD  .year( ), std::chrono::month(v)); }
};

class SqliteOrm : public ITests
{
	std::vector<IncCodeName> codeNames{};
	std::vector<Income     >   incomes{};
	std::vector<uint> months{};
	     uint  currentMonth {};

public:
	virtual
	~SqliteOrm() override = default;
	 SqliteOrm();

	virtual void operator()() override;
};

#ifndef TEST_ODR_VIOLATION
} // namespace
#endif

} // namespace test
