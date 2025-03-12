#pragma once

#include "UA/budget.hpp"

#include <array>

namespace UA::budget::expenses::program {

enum  label_e : size_t                                        { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_CONS_MB_PK , COD_CONS_MB_PK_NAME , COD_CONS_MB_FK , COD_CONS_MB_FK_NAME , ZAT_AMT , PLANS_AMT , FAKT_AMT, LABEL_COUNT_ }; // e - enum
inline constexpr std::array<    type_e , LABEL_COUNT_> labelT { DATE_/*YM*/, CHAR_    , UINT64_    , UINT16_        , STRING_             , UINT16_        , STRING_             , MONEY_  , MONEY_    , MONEY_  ,/*, PERCENT*/ }; // T - type
inline constexpr std::array<    size_t , LABEL_COUNT_> labelP { REP_PERIOD , FUND_TYP , COD_BUDGET , COD_CONS_MB_PK , COD_CONS_MB_PK_NAME , COD_CONS_MB_FK , COD_CONS_MB_FK_NAME , ZAT_AMT , PLANS_AMT , FAKT_AMT /*, DCYP   */ }; // P - pos
inline constexpr std::array<const char*, LABEL_COUNT_> labelS {"REP_PERIOD","FUND_TYP","COD_BUDGET","COD_CONS_MB_PK","COD_CONS_MB_PK_NAME","COD_CONS_MB_FK","COD_CONS_MB_FK_NAME","ZAT_AMT","PLANS_AMT","FAKT_AMT"/*,DCYP    */ }; // S - str
inline constexpr std::array<const char*, LABEL_COUNT_> labelD { // D - description
	/* REP_PERIOD          */ "Місяць та рік"                                                                                ,
	/* FUND_TYP            */ "Тип фонду"                                                                                    ,
	/* COD_BUDGET          */ "Код бюджету"                                                                                  ,
	/* COD_CONS_MB_PK      */ "Код програмної класифікації (КПК)"                                                            ,
	/* COD_CONS_MB_PK_NAME */ "Найменування КПК"                                                                             ,
	/* COD_CONS_MB_FK      */ "Код функціональної класифікації (КФК)"                                                        ,
	/* COD_CONS_MB_FK_NAME */ "Найменування КФК"                                                                             ,
	/* COD_CONS_EK         */ //"Код економічної класифікації (КЕК)"                                                           , // not in csv
	/* COD_CONS_EK_NAME    */ //"Найменування КЕК"                                                                             , // not in csv
	/* ZAT_AMT             */ "Розпис на рік з урахуванням змін"                                                             ,
	/* PLANS_AMT           */ "Кошторисні призначення на рік з урахуванням змін"                                             ,
	/* FAKT_AMT            */ "Виконано за період"                                                                           ,
	/* FAKT_V2MB_AMT       */ //"Виконано за період (в т.ч. на рахунках у банках)"                                             , // no  values
	/* FAKTSIK_AMT         */ //"в т.ч. за іншими коштами спеціального фонду"                                                  , // no  values
	/* FAKTSIK_V2MB_AMT    */ //"в т.ч. за іншими коштами спеціального фонду (в т.ч. на рахунках у банках)"                    , // no  values
	/* FAKTSPP_AMT         */ //"в т.ч. із плати за послуги, що надаються бюджетним установам"                                 , // no  values
	/* FAKTSPP_V2MB_AMT    */ //"в т.ч. із плати за послуги, що надаються бюджетним установам (в т.ч. на рахунках у банках)"   , // no  values
	/* FAKTSID_AMT         */ //"в т.ч. за іншими джерелами власних надходжень бюджетних установ"                              , // no  values
	/* FAKTSID_V2MB_AMT    */ //"в т.ч. за іншими джерелами власних надходжень бюджетних установ (в т.ч. на рахунках у банках)", // not in csv
	/* DONE_CORR_YEAR_PCT  */ //"Виконання до уточненого річного розпису, %"    , // DCYP
};

} // namespace UA::budget::incomes

ASSERT_UA_BUDGET_REP_PERIOD(UA::budget::expenses::program::REP_PERIOD);
