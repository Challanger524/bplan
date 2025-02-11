#pragma once

#include "UA/budget.hpp"

#include <array>

namespace UA::budget::incomes {

enum  label_e : size_t                                        { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT, LABEL_COUNT_ }; // e - enum
inline constexpr std::array<    type_e , LABEL_COUNT_> labelT { DATE_/*YM*/,    CHAR_ ,    UINT64_ ,  UINT32_ ,  STRING_ ,  MONEY_ ,    MONEY_ ,   MONEY_ /*, PERCENT*/ }; // T - type
inline constexpr std::array<    size_t , LABEL_COUNT_> labelP { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_INCO , NAME_INC , ZAT_AMT , PLANS_AMT , FAKT_AMT /*, DCYP   */ }; // P - pos
inline constexpr std::array<const char*, LABEL_COUNT_> labelS {"REP_PERIOD","FUND_TYP","COD_BUDGET","COD_INCO","NAME_INC","ZAT_AMT","PLANS_AMT","FAKT_AMT"/*, DCYP   */ }; // S - str
inline constexpr std::array<const char*, LABEL_COUNT_> labelD { // D - description
	/*         REP_PERIOD */ "Місяць та рік"                                   ,
	/*           FUND_TYP */ "Тип фонду"                                       ,
	/*         COD_BUDGET */ "Код бюджету"                                     ,
	/*         COD_INCO   */ "Код доходу"                                      ,
	/*           NAME_INC */ "Найменування коду доходу"                        ,
	/*            ZAT_AMT */ "Розпис на рік з урахуванням змін"                ,
	/*          PLANS_AMT */ "Кошторисні призначення на рік з урахуванням змін",
	/*           FAKT_AMT */ "Виконано за період"                              ,
	/* DONE_CORR_YEAR_PCT */ //"Виконання до уточненого річного розпису, %"    , // DCYP
};

} // namespace UA::budget::incomes

ASSERT_UA_BUDGET_REP_PERIOD(UA::budget::incomes::REP_PERIOD);


// Stash //

// declaring each variable separately, overcomlicated design
//size_t REP_PERIOD = -1, COD_INCO = -1, NAME_INC = -1, ZAT_AMT = -1, PLANS_AMT = -1, FAKT_AMT = -1;
