#pragma once

#include <rapidcsv.h>

namespace test {

class ITests { // tests interface
public:
	virtual
	~ITests() {}
	 ITests() {}

	virtual void operator()() = 0; // main functor
};

void ReadCsvQ();
void CsvFilterT(rapidcsv::Document *csv_ptr = nullptr); // ImGui window with a `.csv` data convertd into table view

}
