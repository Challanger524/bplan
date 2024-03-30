#include "app/data.hpp"

#include <format>
#include <array>

void TaxUnit::InitChildsParent(const bool full_depth)
{
	// `_ttrl` - traverse tree recurse lambda
	std::function<void(TaxUnit &)>
	InitChildsParent_ttrl = [&InitChildsParent_ttrl, full_depth](TaxUnit &unit) {
		for (auto &u : unit.units) {
			if (u.parent == nullptr) {
				u.parent = &unit;
				InitChildsParent_ttrl(u);
			}
			else if (full_depth)
				InitChildsParent_ttrl(u);
		}
	};

	InitChildsParent_ttrl(*this);
}

void TaxUnit::InitCapitals()
{
	// relation capitals
	constexpr std::array<kind_t, 4> relCapitals {
		kind::rel_hromada,
		kind::rel_rayon_o,
		kind::rel_oblast ,
		kind::rel_country,
	};

	// `_ttrl` - traverse tree recurse lambda
	std::function<TaxUnit*(TaxUnit &, kind_t)>
	FindCapital_ttrl = [&FindCapital_ttrl] (TaxUnit &unit, const kind_t this_capital) -> TaxUnit*
	{ // depth search for a valid capital settelment
		if (unit.is & kind::taxlevel) // below taxlevel
		{
			if (unit.is & this_capital) // found
				return &unit;

			for (auto &u : unit.units)
				if (auto *ptr = FindCapital_ttrl(u, this_capital); ptr != nullptr)
					return ptr;
		}

		return nullptr;
	};

	std::function<void(TaxUnit &)>
	InitCapitals_ttrl = [&](TaxUnit &unit)
	{
		if (!(unit.is & kind::taxlevel))
			return;

		if (unit.capital == nullptr && unit.is & kind::has_capital) {
			kind_t captial_of{};
			for (const auto r : relCapitals) {
				if (unit.is & r) {
					captial_of = (unit.is & kind::has_capital) ^ r;
					break;
				}
			}
			ASSERT(captial_of);

			unit.capital = FindCapital_ttrl(unit, captial_of);
		}

		for (auto &u : unit.units)
			InitCapitals_ttrl(u);
	};

	InitCapitals_ttrl(*this);
}

// -> FormDisplay(TaxUnit &unit, bool init = false)
void TaxUnit::SwitchDisplayLevel(bool just_init)
{
	if (!just_init)
		TaxUnit::display_level = !TaxUnit::display_level;

	auto NumWidth = [](auto s) {
		uint width = 0;
		while (s > 0) {
			s /= 10;
			width++;
		}
		return width;
	};

	// `_ttrl` - traverse tree recurse lambda
	std::function<void(TaxUnit &, int)>
	SwitchDisplayLevel_ttrl = [&SwitchDisplayLevel_ttrl, &NumWidth](TaxUnit &unit, int level) // sets level
	{
		if (!unit.is || unit.units.empty()) return;

		//int nextLevel = level;
		auto &units = unit.units;
		const auto width = NumWidth(units.size() + 1);

		if (units[0].is & kind::a_taxpayer) {
			for (size_t i{}; i < units.size(); i++)
				units[i].display = std::format("{1:{2}}. {0}"    , units[i].name, i+1, width);
		}
		else if (TaxUnit::display_level && !(units[0].is & kind::filter_tl)) {
			for (size_t i{}; i < units.size(); i++)
				units[i].display = std::format("{3}.{1:{2}}. {0}", units[i].name, i+1, width, level);
			level++;
		}
		else {
			for (auto &u : units)
				u.display = u.name;
		}

		for (auto &u : unit.units)
			SwitchDisplayLevel_ttrl(u, level);
	};

	// root
	auto &unit = *this;
	if (TaxUnit::display_level && unit.is & kind::taxlevel)
		unit.display = std::format("{}. {}", 1, unit.name);
	else
		unit.display = unit.name;

	SwitchDisplayLevel_ttrl(*this, 2);
}

template <typename T, typename Alloc>
concept erasable = requires(Alloc m, T *p) { { std::allocator_traits<Alloc>::destroy(m, p) } -> std::same_as<void>; };

void TaxUnit::Test()
{
	static_assert(erasable <TaxUnit, std::vector<TaxUnit>::allocator_type>);
	//if (!name.empty() || is != kind::none) BREAKPOINT();

	TaxUnit::root = this;
	this->is = kind::country;
	this->name = "Україна";

#if 1 // pick move/copy (operators)
	this->units = {
		TaxUnit(kind::oblast, "Чернігівська область", {
			TaxUnit(kind::rayon_o, "Чернігівський район", {
				TaxUnit(kind::filter_tl, "територіальна громада", {
					TaxUnit(kind::filter_tl, "міська", {
						TaxUnit(kind::hromada, "Чернігівська МТГ", {
							TaxUnit(kind::city_capital_whole_oblast, "м.Чернігів", {
								TaxUnit(kind::rayon_c, "Новозаводський район", {
									TaxUnit(kind::physical, "фізичні особи", {
										TaxUnit(kind::workers, "наймані робітники", {
											TaxUnit(kind::worker, "<найманий робітник>")
										}),
										TaxUnit(kind::FOPs, "ФОПи", {
											TaxUnit(kind::FOP, "<ФОП>")
										})
									}),
									TaxUnit(kind::juridical, "юридичні особи", {
										TaxUnit(kind::TOVs, "ТОВ", {}),
										TaxUnit(kind:: PPs, "ПП", {}),
										TaxUnit(kind:: APs, "АП", {}),
										TaxUnit(kind:: NPs, "НП", {}),
										TaxUnit(kind::  Gs, "Г", {})
									})
								}),
								TaxUnit(kind::rayon_c, "Деснянський район", {})
							})
						}),
					}),
					TaxUnit(kind::filter_tl, "селищна", {}),
					TaxUnit(kind::filter_tl, "сільска", {})
				})
			})
		}),
		TaxUnit(kind::oblast, "Сумська область", {}),
		TaxUnit(kind::Kyiv, "м.Київ", {}),
		//TaxUnit(kind::Simferopol, "м.Сімферополь", {}),
		//TaxUnit(kind::region, "Автономна Республіка Крим", {}),
	};

	this->
		units[0]. // 1
		units[0]. // 2
		units[0]. // 3
		units[0]. // 4
		units[0]. // 5
		units[0]. // 6
		units[0]. // 7
		units[0]. // 8
		units[1]. // 9
		units.resize(100'000); // 10

#else
 	//units = {TaxUnit(kind::region, "Чернігівська область")};
 	//units = std::vector<TaxUnit>{{kind::region, "Чернігівська область"}};
 	/* units =
		std::vector<TaxUnit>{
			{kind::region, "Чернігівська область", std::vector<TaxUnit>{
				{kind::rayon_o, "Чернігівський район"}}},
			{kind::region, "Сумська область", std::vector<TaxUnit>{}}
		}; */
 	//units = {TaxUnit(kind::region, "Чернігівська область")};
#endif

	//BREAKPOINT();
	[[maybe_unused]] TaxUnit &itis = *this; // for clang vscode debugging
	this->InitChildsParent();
	this->InitCapitals();

	this->SwitchDisplayLevel();
}
