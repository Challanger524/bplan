#pragma once
//#define HAS_CODECVT
#include <rapidcsv.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace bplan::csv {

namespace rcv = rapidcsv;

void Print(std::filesystem::path csv_fp)
{
	rcv::Document csv(csv_fp.string(),
					  rapidcsv::LabelParams(-1, -1),
					  rapidcsv::SeparatorParams(';')
					 );

	//std::cout << csv.GetCell<std::string>(0uz, 0uz);
	std::string period;
	for (size_t i = 0; i < csv.GetRowCount(); i++) {
		const auto row = csv.GetRow<std::string>(i);

	    if (row.empty()) continue;
		if (row[1] != "T") continue; // Т - разом
		if (period != row[0]) {
			period = row[0];
			std::cout << "\n" << period << ":\n";
		}

		std::cout << "  " << row[3];
		for (size_t j = 4; j < row.size(); j++)
			if (j != 1 && j != 2 && j != 4)
				std::cout << ";" << std::setw(12) << row[j];
		std::cout << std::endl;
	}
}

} // namespace bplan::csv
