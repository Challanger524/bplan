#include "test/tests.hpp"

//#define HAS_CODECVT
#include <rapidcsv.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <assert.h>

namespace rcv = rapidcsv;
namespace fs  = std::filesystem;

namespace test
{

constexpr static auto FPATH_CSV_M = "res/testing/local_2555900000_INCOMES_2023_month.csv";


namespace bplan
{
void CsvFilter_T(rcv::Document &csv) {
	for (size_t i = 0; i < csv.GetRowCount(); /*++*/)
	{
		const auto fund_typ = csv.GetCell<char>("FUND_TYP", i);
		if (fund_typ != 'T') csv.RemoveRow(i); // FUND_TYP=T - разом
		else i++;
	}
}

void CsvFilter_T(const fs::path &in, const fs::path &out) {
	assert(fs::exists(in));
	rcv::Document csv(in.string(), rcv::LabelParams(), rcv::SeparatorParams(';'));
	test::bplan::CsvFilter_T(csv);
	csv.Save(out.string());
}
}


void CsvFilterT(rcv::Document *csv_ptr)
{
	std::cout << "\nTesting: " << FPATH_CSV_M << "\n";

	// Process (shrink) the source .csv:
	const fs::path csvFilePath         = fs::path(FPATH_CSV_M).make_preferred();
	const fs::path csvStem             = csvFilePath.stem();
	const fs::path csvParentFolderPath = csvFilePath.parent_path();
	const fs::path csvSaveFolderPath   = csvParentFolderPath / csvStem;

	//  - create new folder to store processed data: folder name == file name
	if (!fs::exists(csvSaveFolderPath))
	     fs::create_directory(csvSaveFolderPath);

	//  - parse & erase rows that match the predefined options (like: preserve only FUND_TYP=T)
	const fs::path csvFilterName_T = fs::path(csvStem).concat("-T").replace_extension(".csv");
	const fs::path csvFilterPath_T = csvSaveFolderPath / csvFilterName_T;
	assert(fs::exists(csvFilePath));
	bplan::CsvFilter_T(csvFilePath, csvFilterPath_T);

	assert(fs::exists(csvFilterPath_T));
	std::cout << fs::file_size(csvFilePath)     << " B " << csvFilePath     << '\n';
	std::cout << fs::file_size(csvFilterPath_T) << " B " << csvFilterPath_T << '\n';

	// Load and Pass the filtered CSV data via `pointer` to `unique_ptr`
	if (csv_ptr) {
		rcv::Document &csv = *csv_ptr;
		csv.Load(csvFilterPath_T.string(), rcv::LabelParams(), rcv::SeparatorParams(';'));
	}

}

} // test namespace
