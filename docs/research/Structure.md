### Structuring
##### \<admnisnistrative division>
- бюджет адміністративної одиниці (зведений = j)
	- бюджет адміністративної одиниці
	- \<адміністративні під-одиниці>

### Data Structure

#### `Budget`
	- income  - дохід
	- expanse - видаток
	- credit  - кредитування
<!-- expanse: - finance - фінансування -->

#### Budgets of administrative units:

BudAdminUnits: `country`, `oblast`, `rayon`, `hromada`, `settelment` (, `city rayon`), _`filter`_

- `Administrative unit`
- `Budget` of administrative unit (total) = (unit + sub-units)
	- `Budget` of administrative unit
	- `Budget` of `administrative sub-units[]` (totals) (**recurse**)

#### Budgets of `Persons`:

BudPersUnits: `country`, `oblast`, `rayon`, `hromada`, `settelment` (, `city rayon`)

...city `Persons`

- `projects`
- `unemploed`
- `pensioners`
- `Subjects of Entrepreneurial Activity`
	- `physical`   - фіз-особи
		- `workers`    - наймані працівники
		- `FOPs`       - ФОПи
	- `juridicial` - юр-особи
		- `enterprises`           - підприємства
		- `companies`             - товариства
			- `LTDs` (`TOVs`)         -  ТОВ
			- `TDVs`                  -  ТДВ
			-  `PTs`                  - ПТ   - повне
			-  `KTs`                  - КТ   - командитне
			-  `DTs`                  - ДТ   - довірче
			-  `ATs`                  - АТ   - акціонерне
				- `open`                  - відкрите акціонерне тов
				- `closed`                - закрите  акціонерне тов
		- `organizations`         - організації
		- `institution` (service) - установи
		- `establishment`         - заклади

#### Budget GUI layout

- Budget
	- year: **`2024`** `2023` `2022`...
		- item: **`combined`** `incomes` `expenses`:`program`/`functional`
			- quarter: **`year`** `Q1` `Q2` `Q3` `Q4`
				- month: `quarter` `01` `02` `03` - no need
					- last quarter/month table:
		- item:   `combined` **`incomes`** `expenses`:`program`/`functional`
			- quarter: `all` **`Q1`** `Q2` `Q3` ~~`Q4`~~
				- month: `all` **`01`** `02` `03`

- Budget
- Yearof
- BItem: Incomes
- Quarter
- Month
- Income

- years (last quartal/month)
	- year brief: incomes, outcomes
	- year table
	- quarters: q1, q2,...
		- quarter brief
		- quarter detailed
			- months: m01, m02,...m12
				- month brief
				- month detailed

- years (last quartal/month)
	- year brief: incomes, outcomes
	- quarters: q1, q2,...
		- quarter brief
			- months: m01, m02,...m12
				- month brief
	- year table
		- quarters: q1, q2,...
			- quarter detailed
				- months: m01, m02,...m12
					- month detailed

- 25010000 - year (last - 0)
- 	dispatch?
	- 25010100
	- 25010200
-		quarters!:
	-		25010000 - quarter 1
	- 	dispatch?
		- 0,1,2
	- 	months!:
		-	25010000 - month 1
		- 	dispatch?


##### Table

| COD_INCO | FAKT_AMT | ZAT_AMT | PLANS_AMT |
| ---      | ---      | ---     |---        |
| 25010000 | 4295464.34 | 76792759 | 76792759 |

```c++
struct FinnUnit
{
	kind_t is{}; // skip, generic, filter, adminst, budget, subject
	std::string display{};      // display tree-node header

	FinnUnit *parent {nullptr};
	std::vector<FinnUnit> units_finn{0.budget, 1.sub_adminst_1, 2.sub_adminst_2,...}; // Budget + administrative sub-units (budgets)

	// Data: Budget
	Budget *budget; // зведений / звичайний
	//void *data;
	//union {};
	//std::variant<Budget, Subject>
};

// budget L1: 1'0000000 - Податкові надходження
// budget L2: 1'10'00000 - Податки на доходи, податки на прибуток, податки на збільшення ринкової вартості
// budget L3: 1'10'1'0000 - Податок та збір на доходи фізичних осіб
// budget L4: 1'10'1'0100' - Податок на доходи фізичних осіб, що сплачується податковими агентами, із доходів платника податку у вигляді заробітної плати
// budget L4: 1'10'1'0XXX' - Податок...
struct Budget
{
	//kind_t is{};           // filter, budget-level, budget
	//std::string display{}; // display tree-node header

	//BudgUnit *parent {nullptr};
	FinnUnit *parent {nullptr};
	//std::vector<BudgUnit> budgets{}; // administrative sub-units

	//Budget *budget; // budget: accumulated // data
	struct Income {
		date     REP_PERIOD;
		//uint8_t  FUND_TYP; // always T - разом
		uint64_t COD_BUDGET; // 2555900000: 2555900000
		uint32_t COD_INCO;   // ! ! !
		string   NAME_INC;
		money    ZAT_AMT, PLANS_AMT, FAKT_AMT; // ! ! !
		//percent  DONE_CORR_YEAR_PCT;
	};
	std::vector<const Income > incs; //incomes;
	std::vector<const Expanse> exps; //expanses;
	std::vector<const Finance> fins; //finanses/financings;
	std::vector<const Credit > creds; //credits;
	//struct IncomeLvlLast { Income; std::vector<Income>; };
	//struct IncomeLvl { Income; std::vector<IncomeLvl>; };
	//struct BIncClassif { std::vector<IncomeLvl>(5); };
	struct Expanse; // PROGRAM / FUNCTIONAL / ECONOMIC
	struct Financing; // DEBTS / CREDITOR
	struct Credits; // PROGRAM / FUNCTIONAL / CREDIT
};

struct Subject : public Budget // person (phys, jur), project, tender
{
	kind_t is{};           // filter, subject
	//std::string display{}; // display tree-node header

	void *FinnUnit {nullptr};
	//std::vector<FinnUnit> units_finn;

	// Data
	enum Jur_stats {
		Організаційно-правові форми господарювання;
		формою власності;
		формою об’єднання майна і відповідальності господарських товариств: _далі_ товариство..;
		правовим статусом і формою господарювання (статутним фондом);
		метою і характеом діяльності;
		функціональними характеристиками;
		галузео-функціоналним видом діяльності;
		належністю капіталу;
		сферою діяльності;
		технологічною цілісністю і підпорядкованістю;
		об'єднанністю;
		розміром;
	} stats;

	struct metrics;
	struct bplan;   // ! ! !
};

struct AdmnUnit
{
	kind_t is{};
	std::string display{};      // display tree-node header
	std::string display_budd{}; // total budget

	AdmnUnit *parent {nullptr};

	union {
		std::vector<FinnUnit> units_finn{}; // administrative sub-units
		std::vector<FinnUnit> units_pers{}; // administrative sub-units
	}

	BuddUnit *budget_acc; // budget: accumulated
	BuddUnit *budget_adm; // budget: of administrative unit
};

struct BudgUnit
{
	kind_t is{};           // filter, top-budget, budget
	std::string display{}; // display tree-node header

	BudgUnit *parent {nullptr};
	std::vector<BudgUnit> budgets{}; // administrative sub-units

	Budget *budget; // budget: accumulated // data
};
```
