#include "test/tests.hpp" // class ITests

#include <rapidcsv.h>
#include <imgui.h>

#include <span>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <type_traits>
#include <locale>
#include <assert.h>

namespace bplan {
namespace chrono {
inline auto get(std::chrono::year  p) { return p.operator          int(); }
inline auto get(std::chrono::month p) { return p.operator unsigned int(); }
inline auto get(std::chrono::day   p) { return p.operator unsigned int(); }

#if !defined(__clang__) // Clang still has no std::chrono::(parse/from_stream) implementation
inline auto& from_stream(const auto &idate, auto format, auto &odate) { std::istringstream sstream(idate); return std::chrono::from_stream(sstream, format, odate); }
#else // Clang has only particial C++20 Calendar and Time Zone implementation
      // https://github.com/llvm/llvm-project/issues/99982
      // https://libcxx.llvm.org/Status/Cxx20.html#note-p0355:~:text=Extending%20chrono%20to%20Calendars%20and%20Time%20Zones
#include <sstream>
inline auto from_stream(const auto &idate, auto format, auto &odate) {
	static_assert(std::is_same_v<decltype(odate), std::chrono::year_month&>);
	assert(std::string("%m.%Y").compare(format) == 0);

	std::istringstream iss(idate);
	int month{}, year{};

	iss >> month;
	iss.ignore(1, '.'); // example: "04.2023"
	iss >> year;

	odate = std::chrono::year_month(std::chrono::year(year), std::chrono::month(month));
	//std::cout<<"\n "<<odate.month().operator unsigned int()<<"."<<odate.year().operator int();
	return void();
}
#endif // __clang__
}

template<class T> inline std::enable_if_t<!std::is_same_v<T, std::string>, short> Delta(T l, T r) { return l == r ? 0 : (l < r) ? -1 : +1; }
template<class T> inline short Delta              (const std::string & , const std::string & ) { static_assert(false, "custom template specialization required for given type" ); assert(false); return {}; }
// `std::string` that can hold `utf-8` encoding
  template<> inline short Delta<std:: string>(const std::string &l, const std::string &r) { return std::locale().operator()(l, r) ? -1 : std::locale().operator()(r, l) ? +1 : 0; }
//template<> inline short Delta<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 : std::locale().operator()(l, r) ? -1 : +1;  } //! bad == cmp
//template<> inline short Delta<std:: string>(const std::string &l, const std::string &r) { return l == r ? 0 :  bp::locale  .operator()(l, r) ? -1 : +1;  }
//template<> inline short Delta<std::wstring>(const std::string &l, const std::string &r) { return l == r ? 0 :      locale  .operator()(l, r) ? -1 : +1;  }
//template<> inline short Delta<std::string >(const std::string &l, const std::string &r) { return scast<short>(         l.compare  (r));       }
//template<> inline short Delta<char        >(const std::string &l, const std::string &r) { return Delta(l.empty() ? '\0' : l[0], r.empty() ? '\0' : r[0]); }

template<> inline short Delta<uint64_t    >(const std::string &l, const std::string &r) { return Delta(std::stoull(l), std::stoull(r));       }
template<> inline short Delta<ldouble     >(const std::string &l, const std::string &r) { return Delta(std::stold (l), std::stold (r));       }
template<class Date> short Delta           (const std::string &l, const std::string &r, auto fromat) { // `Date` suitable for: std::chrono:: `day`, `year_month`, ...
	Date dl;
	Date dr;
	//std::istringstream sstream;
	//sstream.clear(); sstream.str(l); sstream >> std::chrono::parse(fromat, dl);
	//sstream.clear(); sstream.str(r); sstream >> std::chrono::parse(fromat, dr);
	//sstream.clear(); sstream.str(l); std::chrono::from_stream(sstream, fromat, dl);
	//sstream.clear(); sstream.str(r); std::chrono::from_stream(sstream, fromat, dr);
	bp::chrono::from_stream(l, fromat, dl);
	bp::chrono::from_stream(r, fromat, dr);
	return Delta(dl, dr);
}

// STL wrapper that accepts `container` instead of `begin()`, `end()` range iterators, YAY
constexpr inline auto sort(auto &container, auto comparator = std::less<>{}) { return std::sort(container.begin(), container.end(), comparator); }
} // namespace bplan

//template <class T> using undtp = class std::underlying_type_t<T>;

namespace budget {
enum  type_e : short { CHAR, UINT32, UINT64, DATE/*YM*/, STRING/*, TEXT*/, MONEY/*, PERCENT*/, TYPE_COUNT_ }; // enum of types (across budget CSVs)
//using labelT = std::tuple               < std::chrono::year_month ,     char ,   uint64_t , uint32_t ,std::string, money_t,   money_t ,  money_t>; // T - type (tuple)
} // namespace gov

namespace budget::income {
using namespace budget;

enum  labe_e : size_t                                  { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT, LABEL_COUNT_ }; // E - enum
inline constexpr std::array<    size_t , LABEL_COUNT_> labelP { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT /*, PERCENT*/ }; // P - pos
inline constexpr std::array<    type_e , LABEL_COUNT_> labelT { DATE/*YM*/ ,     CHAR ,     UINT64 ,   UINT32 ,   STRING ,   MONEY ,     MONEY ,    MONEY /*, PERCENT*/ }; // T - type
inline constexpr std::array<const char*, LABEL_COUNT_> labelS {"REP_PERIOD","FUND_TYP","COD_BUDGET","COD_INCO","NAME_INC","ZAT_AMT","PLANS_AMT","FAKT_AMT"/*, DCORRYP*/ }; // S - str
inline constexpr std::array<const char*, LABEL_COUNT_> labelD { // D - description
	/*         REP_PERIOD */ "Місяць та рік",
	/*           FUND_TYP */ "Тип фонду",
	/*         COD_BUDGET */ "Код бюджету",
	/*         COD_INCO   */ "Код доходу",
	/*           NAME_INC */ "Найменування коду доходу",
	/*            ZAT_AMT */ "Розпис на рік з урахуванням змін",
	/*          PLANS_AMT */ "Кошторисні призначення на рік з урахуванням змін",
	/*           FAKT_AMT */ "Виконано за період",
	/* DONE_CORR_YEAR_PCT */ //"Виконання до уточненого річного розпису, %", // DCORRYP - DONE_CORR_YEAR_PCT
};

} // namespace budget::income

namespace bud = budget;

namespace bplan
{
struct SortSpec {
	size_t column{};
	bool   ascend{true};
};

using sort_specs_t = std::vector<SortSpec>;
using     vecstr_t = std::vector<std::string>;

inline void SortBudget(std::vector<vecstr_t> &table, std::span<const SortSpec>  specs, std::span<const bud::type_e> labelT)
{
	/*
	std::sort(table.begin(), table.end(), [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs)*/
	 bp::sort(table                     , [&labelT, &specs](const vecstr_t &lhs, const vecstr_t &rhs){
		for (const auto &spec : specs)
		{
			//if (lhs[column] == rhs[column]) continue; // easy to check any values for equality (coz string), but rare and resource consuming case

			short delta{};
			const auto    &column = spec.column; // real column `idx` from the main csv storage
			switch (labelT[column])
			{
				case bud::UINT32   :
				case bud::UINT64   : delta = bp::Delta<               uint64_t>(lhs[column], rhs[column]); break;
				case bud::MONEY    : delta = bp::Delta<                money_t>(lhs[column], rhs[column]); break;
				case bud::CHAR     :
				case bud::STRING   : delta = bp::Delta<std::            string>(lhs[column], rhs[column]); break;
				case bud::DATE     : delta = bp::Delta<std::chrono::year_month>(lhs[column], rhs[column], "%m.%Y"); break;
				default       : assert(false && "individual switch's case option for each 'type_e' required"); break;
			}

			if (delta == 0 ) continue; // compare with another column specified in next `spec` : specs
			if (spec.ascend) return delta < 0;
			else             return delta > 0;
		}

		return false; // means all Specs are equal and no need to sort
	});
}
inline void SortBudget(rapidcsv::Document    &  csv, std::span<const SortSpec>  specs, std::span<const bud::type_e> labelT)
{
	const size_t csvSize [[maybe_unused]] = csv.GetRowCount();

	std::vector<bp::vecstr_t> table; // moved csv table
	table.reserve(csvSize);

	while (csv.GetRowCount() > 0) { // "move" all data from csv
		table.push_back(csv.GetRow<std::string>(0));
		csv.RemoveRow(0);
	}

	bp::SortBudget(table, specs, labelT);

	//for (const auto &ch : table) { csv.InsertRow<std::string>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::remove_const_t<std::remove_pointer_t<decltype(ch.data())>>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::decay_t<decltype(*std::begin(ch))>>(row++, ch); } // dereferencing, not good
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(ch.begin())>>(row++, ch); }
	//for (const auto &ch : table) { csv.InsertRow<std::iter_value_t<decltype(std::begin(ch))>>(row++, ch); } // final leap of solution evolution
	for (size_t i = 0; i < table.size(); i++) { csv.InsertRow<std::string>(i, table[i]); }

	assert(csvSize == csv.GetRowCount());
}
inline void SortBudget(rapidcsv::Document    &  csv, const ImGuiTableSortSpecs *specs, std::span<const bud::type_e> labelT, std::span<const size_t> labelV, std::span<const size_t> LabelIforDateSort = std::span<const size_t, 0>())
{
	using namespace budget;
	using namespace budget::income;
	assert(specs != nullptr);
	std::span<const ImGuiTableColumnSortSpecs> specsSpan(specs->Specs, specs->SpecsCount);

	bp::sort_specs_t specsSort;
	specsSort.reserve(specsSpan.size() + (LabelIforDateSort.empty() ? 0 : 1));

	if (!LabelIforDateSort.empty()) specsSort.push_back(bp::SortSpec{.column = LabelIforDateSort[REP_PERIOD], .ascend = true});
	for (size_t i = 0; i < specsSpan.size(); i++)
		specsSort.push_back(bp::SortSpec {.column = labelV[specsSpan[i].ColumnIndex],
							              .ascend =        specsSpan[i].SortDirection == ImGuiSortDirection_Ascending});

	SortBudget(csv, specsSort, labelT);
}

} // namespace bplan

namespace test {

class CsvTable : public ITests
{
	rapidcsv::Document csv{};

	//size_t REP_PERIOD = -1, COD_INCO = -1, NAME_INC = -1, ZAT_AMT = -1, PLANS_AMT = -1, FAKT_AMT = -1; // declaring each variable separately design
	std::array<size_t, budget::income::LABEL_COUNT_> labelI{}; // I - id positions (real)

public:
	virtual
	~CsvTable() override = default;
	 CsvTable();

	virtual void operator()() override;
};

} // namespace test
