#include "app/data.hpp"
#include "settings.hpp"

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
		bp::sett.disp.depthLevel = !bp::sett.disp.depthLevel;

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
		else if (bp::sett.disp.depthLevel && !(units[0].is & kind::filter_tl)) {
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
	if (bp::sett.disp.depthLevel && unit.is & kind::taxlevel)
		unit.display = std::format("{}. {}", 1, unit.name);
	else
		unit.display = unit.name;

	SwitchDisplayLevel_ttrl(*this, 2);
}
