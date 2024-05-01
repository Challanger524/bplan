#pragma once

#include <type_traits>
#include <functional>
#include <vector>
#include <string>
#include <iostream>

// Tax Unit Kind
namespace tuKind {
enum TaxUnitKind : ulong
{
	none             = 0,
	generic          = 1ull << 0,

	// tax level          // administrative divisions
		filter_tl = 1ull << 5, // tax level filter - фільтр: громад
		rayon_c = 1ull <<  6, // район (міста)
		farm    = 1ull <<  7, // село
		village = 1ull <<  8, // селище
		city    = 1ull <<  9, // місто
		hromada = 1ull << 10, // міська/селищна/сільська територіальна громада
		rayon_o = 1ull << 11, // район (області)
		oblast  = 1ull << 13, // область - 24 областей
		region  = 1ull << 12, // регіон  - 27 регіонів: область/Автономна Республіка Крим/місто зі спеціальним статусом (Київ, Севастополь)
		country = 1ull << 14, // країна
			//rayon_c = a_taxlevel | a_rayon_c, // район (міста)
			//farm    = a_taxlevel | a_farm   , // село
			//village = a_taxlevel | a_village, // селище
			//city    = a_taxlevel | a_city   ,
			//hromada = a_taxlevel | a_hromada, // міська/селищна/сільська територіальна громада
			//rayon_o = a_taxlevel | a_rayon_o, // район (області)
			//region  = a_taxlevel | a_region , // 27 регіонів: область/Автономна Республіка Крим/місто зі спеціальним статусом (Київ, Севастополь)
			//oblast  = a_taxlevel | a_oblast | a_region, // 24 областей
			//country = a_taxlevel | a_country,

			special = region | city, // special city: Київ, Севастополь
			//filter_tl = a_taxlevel | a_filter_taxlevel,

	taxlevel = rayon_c|farm|village|city|hromada|rayon_o|region|oblast|country|special|filter_tl,

	//a_taxelem  = 1ull << 16,
	filter_tp  = 1ull << 17, // tax payer filter
	a_taxpayer = 1ull << 18,
		a_physical  = 1ull << 19, // ФОПи / найманиі робітники
		a_juridical = 1ull << 20, // організації / підприємства / товариства
			a_workers = 1ull << 22, //       найманий робітник
			a_FOPs    = 1ull << 22, // ФОП - фізична особа підприємець
			a_TOVs    = 1ull << 23, // ТОВ - товариства з обмеженою відповідальністю
			a_PPs     = 1ull << 24, //  ПП - приватні підприємства (схожі на ТОВ)
			a_APs     = 1ull << 25, //  АП - акціонерні підприємства
			a_NPs     = 1ull << 26, //  НП - неприбуткові організації
			a_Gs      = 1ull << 27, //   Г - громадські   організації
				//a_worker = 1ull << N, //       найманий робітник
				//a_FOP    = 1ull << N, // ФОП - фізична особа підприємець
				//a_TOV    = 1ull << N, // ТОВ -   товариство
				//a_PP     = 1ull << N, //  ПП - підприємство
				//a_AP     = 1ull << N, //  АП - підприємство
				//a_NP     = 1ull << N, //  НП - організація
				//a_G      = 1ull << N, //   Г - організація

		physical  =      a_physical,  // фіз-особи
			workers =    a_physical  | a_workers, // наймані робітники
			FOPs    =    a_physical  | a_FOPs,    // ФОПи - фізичні особи підприємеці
				worker = a_physical  | a_workers | a_taxpayer, // найманий робітник
				FOP    = a_physical  | a_FOPs    | a_taxpayer, // ФОП - фізична особа підприємець
		juridical =      a_juridical, // організації / підприємства / товариства
			TOVs    =    a_juridical | a_TOVs, // ТОВ -   товариства з обмеженою відповідальністю
			PPs     =    a_juridical | a_PPs , //  ПП -     приватні підприємства (схожі на ТОВ)
			APs     =    a_juridical | a_APs , //  АП -   акціонерні підприємства
			NPs     =    a_juridical | a_NPs , //  НП - неприбуткові організації
			Gs      =    a_juridical | a_Gs  , //   Г -   громадські організації
				TOV    = a_juridical | a_TOVs    | a_taxpayer, // ТОВ - товариство з обмеженою відповідальністю
				PP     = a_juridical | a_PPs     | a_taxpayer, //  ПП - приватне    підприємство (схоже на ТОВ)
				AP     = a_juridical | a_APs     | a_taxpayer, //  АП - акціонерне  підприємство
				NP     = a_juridical | a_NPs     | a_taxpayer, //  НП - неприбуткова організація
				G      = a_juridical | a_Gs      | a_taxpayer, //   Г - громадська   організація

					taxelem = worker|FOP|TOV|PP|AP|NP|G| filter_tp,

	filter = filter_tl | filter_tp, // `filter_tl|filter_tp`

	// Capital
		capital_of_a_country = 1ull << 36,
		capital_of_a_oblast  = 1ull << 37,
		capital_of_a_rayon   = 1ull << 38,
		capital_of_a_hromada = 1ull << 39,
		//capital_of_a_region  = 1ull << 40,
			capital = capital_of_a_country | capital_of_a_oblast | capital_of_a_rayon | capital_of_a_hromada,

			has_capital = country /*| a_region*/ | oblast | rayon_o | hromada,
			can_be_capital = city | village | farm,

			//capitof_whole_oblast = capital_of_oblast | capital_of_rayon | capital_of_hromada, // capital of the whole oblast+rayon+hromada
			//capitof_whole_rayon  =                     capital_of_rayon | capital_of_hromada,
			capitof_whole_oblast = capital_of_a_oblast | capital_of_a_rayon | capital_of_a_hromada, // capital of the whole oblast+rayon+hromada
			capitof_whole_rayon  =                       capital_of_a_rayon | capital_of_a_hromada,

			city_capital_whole_oblast = city | capitof_whole_oblast,

			rel_country = country | capital_of_a_country,
			rel_oblast  = oblast  | capital_of_a_oblast ,
			rel_rayon_o = rayon_o | capital_of_a_rayon  ,
			rel_hromada = hromada | capital_of_a_hromada,
			//rel_region = a_region | capital_of_a_region ,

	Kyiv = region | special | city | capital_of_a_country | capital_of_a_oblast,
	//Simferopol = region | special | city | capital_of_region  | capital_of_rayon ,
}; // 30 unique flags

using tuKind_t = std::underlying_type_t<TaxUnitKind>;
}

// Gereralized administrative tax unit
struct TaxUnit
{
	using kind = tuKind::TaxUnitKind; // (re)namespace
	using kind_t = std::underlying_type_t<kind>;
	using uidc_t = uint; // tax гid (counter)

public:
	uidc_t id{};
	kind_t is{};
	std::string display{};
	std::string name{};
	std::vector<TaxUnit> units{};

	TaxUnit *parent {nullptr};
	TaxUnit *capital{nullptr};

	// sum
	// diff //?
	// str: sum_round

public:
	//~TaxUnit() {}
	TaxUnit() {}
	TaxUnit(kind_t kind, std::string name, std::vector<TaxUnit> units = std::vector<TaxUnit>()) : is(kind), units(units), name(name) {
		std::cout << ": " << name << std::endl;
	}

	TaxUnit           (const TaxUnit &) = default;
	TaxUnit& operator=(const TaxUnit &that) { if (this != &that) {std::destroy_at(this); std::construct_at(this, that);} return *this; }
	TaxUnit           (TaxUnit &&) = default;
	TaxUnit& operator=(TaxUnit &&) = default; // delete

	//std::underlying_type_t<TaxUnitKind> Is() { return scast<std::underlying_type_t<TaxUnitKind>>(this->is); }
	//explicit operator const char *() { return reinterpret_cast<const char *>(this->display.c_str()); }
	//explicit operator const char *() { return reinterpret_cast<const char *>(!this->display.empty() ? this->display.c_str() : this->name.c_str()); }
	explicit operator const char *() const { return reinterpret_cast<const char *>(this->display.c_str()); }
	explicit operator const void *() const { return reinterpret_cast<const void *>(this); }

public:
	void InitChildsParent(bool full_depth = false);
	void InitCapitals();

	void SwitchDisplayLevel(bool just_init = false);
};

#include <type_traits>
template <class T, class Alloc>
concept erasable = requires(Alloc a, T *t) { { std::allocator_traits<Alloc>::destroy(a, t) } -> std::same_as<void>; };
static_assert(erasable<TaxUnit, std::vector<TaxUnit>::allocator_type>);
