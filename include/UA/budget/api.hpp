#pragma once

#include "bplan/lenof.hpp"
#include <array>

namespace UA::budget {


constexpr inline auto FS_DOWNLOADS = "Downloads/";

constexpr inline auto API_HOST = "api.openbudget.gov.ua";

constexpr inline auto API_PATH_BASE = "/api/public/";
constexpr inline auto API_PATH_PING         = "ping"           ;

//constexpr inline auto API_PATH_LOC_BUD_DD  = "localBudgetDocDay"  ; // supported not planned
//constexpr inline auto API_PATH_LOC_BUD_REP = "localBudgetReport"  ; // supported not planned
//constexpr inline auto API_PATH_LOC_BUD_LL  = "localBudgetLastLoad"; // supported not planned


constexpr inline auto QUERY_BCODE   = "budgetCode"        ;
constexpr inline auto QUERY_BITEM   = "budgetItem"        ;
constexpr inline auto QUERY_BCLASS  = "classificationType";
constexpr inline auto QUERY_BPERIOD = "period"            ;
constexpr inline auto QUERY_BYEAR   = "year"              ;


// Tokens for collecting input and composing: 'filename', 'url query'
enum                        indicator_e     : int { LOCAL           ,  GENERAL     }; // budget indicator
inline constexpr std::array indicators      =     {"local"          , "general"    }; // budget indicator // c-array for imgui tab item api
inline constexpr std::array indicatorsQuery =     {"localBudgetData", "generalData"}; // budget indicator for web api query target


enum                         item_e  : int { INCOMES ,  EXPENSES ,  CREDITS ,  FINANCING_DEBTS ,  FINANCING_CREDITOR }; // budget type
inline constexpr const char* items[] =     {"INCOMES", "EXPENSES", "CREDITS", "FINANCING_DEBTS", "FINANCING_CREDITOR"}; // budget type // c-array for imgui combo (drop list) api

enum                        classif_e : int { PROGRAM ,  FUNCTIONAL ,  ECONOMICS ,  CREDIT }; // classification type
inline constexpr std::array classifs  =     {"PROGRAM", "FUNCTIONAL", "ECONOMICS", "CREDIT"}; // classification type

enum                        period_e   : int { MONTH ,  QUARTER }; // data period
inline constexpr std::array periods    =     {"MONTH", "QUARTER"}; // data period (upper-case)
inline constexpr std::array periodsLow =     {"month", "quarter"}; // data period (lower-case)


#define           DEF_BUD_CODE_INIT         "0000000000"   // 10 digit '\0' terminated budget init string value define (not usable)
inline constexpr auto BUD_CODE_INIT      =  "0000000000" ; // 10 digit '\0' terminated budget init string value
inline constexpr auto BUD_CODE_INIT_LIST = {"0000000000"}; // 10 digit '\0' terminated budget init string value

inline constexpr auto  BUD_CODE_S_55   = "00000 00000"  ; // 10 digit budget init split string value
inline constexpr auto  BUD_CODE_S_235  = "00 000 00000" ; // 10 digit budget init split string value
inline constexpr auto  BUD_CODE_S_2323 = "00 000 00 000"; // 10 digit budget init split string value

#define           DEF_BUD_CODE_HRO_CHE      "2555900000"  // 10 digit budget code of Chernihiv hromada
#define           DEF_BUD_CODE_HRO_CHE_HALF "25559"       //  5 first digit budget code of Chernihiv hromada
inline constexpr auto BUD_CODE_HRO_CHE    = "2555900000"; // 10 digit budget code of Chernihiv hromada
inline constexpr auto BUD_CODE_OBL_KYIV   = "1000000000"; // Kyiv oblast total budget code (min value)
inline constexpr auto BUD_CODE_UKRAINE    = "9900000000"; // Ukraine           budget code (max value)

inline constexpr auto BUD_CODE_DIGIT_COUNT  = 10ull;                  // number of digits in budget code
inline constexpr auto BUD_CODE_STRLEN_MIN   = lenof(BUD_CODE_INIT  ); // min size of stringed budget code

inline constexpr auto BUD_CODE_STRLEN_S55   = lenof(BUD_CODE_S_55  ); // min size of stringed splitted budget code
inline constexpr auto BUD_CODE_STRLEN_S235  = lenof(BUD_CODE_S_235 ); // min size of stringed splitted budget code
inline constexpr auto BUD_CODE_STRLEN_S2323 = lenof(BUD_CODE_S_2323); // min size of stringed splitted budget code


inline constexpr int BUD_YEAR_MIN = 2018; // budget year min
inline constexpr int BUD_YEAR_MAX = 2042; // budget year max

struct budget
{
	static constexpr auto BUFF_BCODE_SIZE = BUD_CODE_STRLEN_MIN + 1;

	int indicator = {LOCAL};

	char code[BUFF_BCODE_SIZE] {DEF_BUD_CODE_HRO_CHE};
	int  item    {INCOMES};
	int  classif {PROGRAM};
	int  period  {MONTH  };
	int  year    {2025   };

	static_assert(BUFF_BCODE_SIZE > BUD_CODE_DIGIT_COUNT);
};


namespace test_static::budget_api_hpp {

static_assert(BUD_CODE_DIGIT_COUNT == sizeof(DEF_BUD_CODE_INIT) - 1);
static_assert(BUD_CODE_DIGIT_COUNT == BUD_CODE_STRLEN_MIN);

}

} // namespace UA::budget


// Stash //
