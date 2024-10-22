#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include <string>

//struct TaxUnit; //FinUnit;

//using std::chrono::year;

struct Budget
{
	uint64_t bcode{}; // budget code
	//TaxUnit *parent {nullptr};

	struct Yearof {
		//using year_t = std::chrono::year;
		std::chrono::year year{/*2024*/};
		money total{}; // total sum of all budget types (incomes + expanses + ...)
		//bool quarter{false};

		//std::string display;
		//char display[sizeof("2024")] = {'\0'};

		template <class T> struct Periodof {
			money total{};
			//using year_month_t = std::chrono::year_month;
			//year_month_t period{};
			std::chrono::year_month period{};
			std::vector<T> table{};

			//std::string display;
			//char display[sizeof("2024.12")] = {'\0'};
		};

		struct IncomesQ {
			struct IncomesM {
				struct Income {
					//date     REP_PERIOD; // Місяць та рік (moved up)
					//uint8_t  FUND_TYP; // Тип фонду (always T - разом)
					//uint64_t COD_BUDGET; // Код бюджету (moved up)
					uint32_t icode{}; // COD_INCO; // Код доходу
					std::string   NAME_INC{};   // Найменування коду доходу // globalize?
					money appr{}; //   ZAT_AMT{}; // Розпис на рік з урахуванням змін
					money plan{}; // PLANS_AMT{}; // Кошторисні призначення на рік з урахуванням змін
					money fact{}; //  FAKT_AMT{}; // Виконано за період
					//percent  DONE_CORR_YEAR_PCT;
				};

				money total{};
				std::vector<Periodof<Income>> incomes{/*3*/};
				//std::string display;
				};
			money total{};
			std::vector<Periodof<IncomesM>> incomesM{/*4*/};
			std::vector<IncomesM::Income> incomes{}; // for a quarter

		public:
			void Load();
		};

		std::vector<IncomesQ> incomesQ{/*4*/};
		std::vector<IncomesQ::IncomesM::Income> incomes{}; // for a year
		#if 0
		std::vector<const Expanse> exps; //expanses;
		std::vector<const Finance> fins; //finanses/financings;
		std::vector<const Credit > creds; //credits;
		//struct IncomeLvlLast { Income; std::vector<Income>; };
		//struct IncomeLvl { Income; std::vector<IncomeLvl>; };
		//struct BIncClassif { std::vector<IncomeLvl>(5); };
		struct Expanse; // PROGRAM / FUNCTIONAL / ECONOMIC
		struct Financing; // DEBTS / CREDITOR
		struct Credits; // PROGRAM / FUNCTIONAL / CREDIT
		#endif
	};

	std::vector<Yearof> budgetY{/*1+*/};
public:
	void Edit();
	void Load(){} // read/download/parse/filter budget tables from filesystem/publicAPI
};

#if 0
struct Budget
{
	uint bcode;

	struct Year {
		std::chrono::year year;
		struct Budgets {
			struct Incomes  ;
			struct Expanses ; //: PROGRAM, FUNCTIONAL, ECONOMIC
			struct Credits  ; //: PROGRAM, FUNCTIONAL, CREDIT
			struct Financing; //: DEBTS  , CREDITOR
		} budgets;
	} years[];
};

struct Incomes {
	struct PeriodQ { //?
		struct PeriodM {
			struct Budget b[]; //fields with data
		} month[];
		struct Budget b;
	} quarter[];
	struct Budget b;
};
#endif
