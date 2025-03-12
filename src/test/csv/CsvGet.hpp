#ifndef TEST_CSV_CSV_GET_HPP_
#define TEST_CSV_CSV_GET_HPP_
#else  // include anti-guard
#error "re-include of non-library header"
#endif // https://stackoverflow.com/a/32628153/11753532

#include "test/tests.hpp"

#include "UA/budget/api.hpp"

#include <rapidcsv.h>

namespace test {

class CsvGet : public ITests
{
	static constexpr auto NAME_MODAL_INPUT = "BudgetIndicators";
	using DrawTableMethodS = void(const rapidcsv::Document&);

private:
	UA::budget::budget budget{.code=DEF_BUD_CODE_HRO_CHE_HALF, .item=UA::budget::INCOMES};
	//UA::budget::item_e budtype{};
	rapidcsv::Document csv{};

	DrawTableMethodS *DrawCsvTableS{};

public:
	virtual
	~CsvGet() override = default;
	 CsvGet();

	virtual void operator()() override;

	void DrawModalInput();
};

} // namespace test
