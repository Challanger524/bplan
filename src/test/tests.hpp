#pragma once

#include <rapidcsv.h>

namespace test {

void ReadCsvQ();
void CsvFilterT(rapidcsv::Document *csv_ptr = nullptr); // ImGui window with a `.csv` data convertd into table view

}
