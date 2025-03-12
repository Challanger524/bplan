#pragma once

#include "UA/budget.hpp"

#include <array>

namespace UA::budget::general::incomes {

enum  label_e : size_t                                        { REP_PERIOD , BUDG_TYP , FUND_TYP , COD_INCO , COD_INCO_NAME , PLAN_BEGIN_YEAR_AMT , PLAN_CORR_YEAR_AMT , PLAN_CORR_PERIOD_AMT , DONE_PERIOD_AMT , LABEL_COUNT_ }; // e - enum
inline constexpr std::array<    type_e , LABEL_COUNT_> labelT { DATE_/*YM*/, CHAR_    , CHAR_    , UINT32_  , STRING_       , MONEY_              , MONEY_             , MONEY_               , MONEY_          }; // T - type
inline constexpr std::array<    size_t , LABEL_COUNT_> labelP { REP_PERIOD , BUDG_TYP , FUND_TYP , COD_INCO , COD_INCO_NAME , PLAN_BEGIN_YEAR_AMT , PLAN_CORR_YEAR_AMT , PLAN_CORR_PERIOD_AMT , DONE_PERIOD_AMT }; // P - pos
inline constexpr std::array<const char*, LABEL_COUNT_> labelS {"REP_PERIOD","BUDG_TYP","FUND_TYP","COD_INCO","COD_INCO_NAME","PLAN_BEGIN_YEAR_AMT","PLAN_CORR_YEAR_AMT","PLAN_CORR_PERIOD_AMT","DONE_PERIOD_AMT"}; // S - str
inline constexpr std::array<const char*, LABEL_COUNT_> labelD { // D - description
	/* REP_PERIOD                  */ "Місяць та рік"                                ,
	/* BUDG_TYP                    */ "Тип бюджету"                                  ,
	/* FUND_TYP                    */ "Тип фонду"                                    ,
	/* COD_INCO                    */ "Код доходу"                                   ,
	/* COD_INCO_NAME               */ "Назва видатків"                               ,
	/* PLAN_BEGIN_YEAR_AMT         */ "Початковий річний план"                       , // many blank/empty values, but not all
	/* PLAN_CORR_YEAR_AMT          */ "Уточнений річний план"                        ,
	/* PLAN_CORR_PERIOD_AMT        */ "Уточнений план за період "                    , // many blank/empty values, but not all
	/* DONE_PERIOD_AMT             */ "Виконано за період"                           ,
	/* DECLIN_PLAN_CORR_PERIOD_AMT */ //"Відхилення до уточненого плану за період, +/-", // not in csv
	/* DONE_PLAN_CORR_PERIOD_PCT   */ //"Виконання до уточненого плану за період,%"    , // not in csv
	/* DONE_PLAN_CORR_YEAR_PCT     */ //"Виконання до уточненого річного плану, %"     , // not in csv
};

} // namespace UA::budget::incomes

ASSERT_UA_BUDGET_REP_PERIOD(UA::budget::general::incomes::REP_PERIOD);
