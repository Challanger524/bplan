#ifndef TEST_CSV_CSV_GET_HPP_
#define TEST_CSV_CSV_GET_HPP_
#else  // include anti-guard
#error "re-include of non-library header"
#endif // https://stackoverflow.com/a/32628153/11753532

#include "test/tests.hpp"

namespace test {

class CsvGet : public ITests
{
public:
	virtual
	~CsvGet() override = default;
	 CsvGet();

	virtual void operator()() override;
};

} // namespace test
