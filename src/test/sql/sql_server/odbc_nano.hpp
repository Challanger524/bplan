#ifndef TEST_SQL_SQL_SERVER_ODBC_NANO_HPP_
#define TEST_SQL_SQL_SERVER_ODBC_NANO_HPP_
#else  //  include anti-guard
#error "re-include of non-library header"
#endif //  https://stackoverflow.com/a/32628153/11753532

#include "test/tests.hpp"

#include <nanodbc/nanodbc.h>

#include <memory>
#include <chrono>
#include <vector>
#include <string>


namespace test::odbc_nano { // "nanodbc"


struct IncCodeName {
	      int         id{-1};
	      int     COD_INCO{};
	std::string   NAME_INC{}; // text translation
};

struct Income {
	std::        string     COD_BUDGET{};
	std::chrono::year_month REP_PERIOD{};
	uint32_t COD_INCO{};
	 int     NAME_INC{}; // foreign key (int)
	double   ZAT_AMT{};
	double PLANS_AMT{};
	double  FAKT_AMT{};

public:
	int  GetYear () const { return REP_PERIOD. year().operator          int(); }
	uint GetMonth() const { return REP_PERIOD.month().operator unsigned int(); }
	void SetYear ( int v) { REP_PERIOD = std::chrono::year_month(std::chrono::year(v), REP_PERIOD  .month( )); }
	void SetMonth(uint v) { REP_PERIOD = std::chrono::year_month(REP_PERIOD  .year( ), std::chrono::month(v)); }
};

class SqlServer : public ITests
{
	nanodbc::connection con{};

	std::vector<IncCodeName> codeNames{};
	std::vector<Income     >   incomes{};
	std::vector<uint> months{};
	     uint  currentMonth {};

public:
	virtual
	~SqlServer() override = default;
	 SqlServer();

	virtual void operator()() override;
};


} // namespace test::sql_server
