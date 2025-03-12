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

	UA::budget::budget budget{.code=DEF_BUD_CODE_HRO_CHE_HALF};
	rapidcsv::Document csv{};

public:
	virtual
	~CsvGet() override = default;
	 CsvGet();

	virtual void operator()() override;

	void DrawModalInput();
};

} // namespace test
