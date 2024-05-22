#include "app/data.hpp"
#include "settings.hpp"

#include <functional>
#include <format>
#include <array>
#include <assert.h>

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
		if (unit.is & kind::administ) // below administ
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
		if (!(unit.is & kind::administ))
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
void TaxUnit::FormDisplay(const ushort level, const size_t pos, const size_t pos_padd)
{
	//if (!is) return;

	this->display.clear();
	this->display.reserve(this->name.size());

	if (!(this->is & kind::filter))
	{
		// depth level
		if (this->is & kind::administ && bp::sett.disp.depthLevel && level > 0)
			this->display += std::format("{}.", level);

		// enumeration
		auto Enumer = [&] () { this->display += std::format("{:{}}.", pos, pos_padd); };
		if (this->is & kind::a_taxpayer) { if (bp::sett.disp.elemEnumer)                   Enumer(); }
		else                             { if (bp::sett.disp.nodeEnumer && pos != 0uz - 1) Enumer(); }
	}

	if (!this->display.empty())
		this->display += ' ';

	this->display += this->name;
}

void TaxUnit::Display()
{
	auto DigitCount_l = [](auto decimal) -> uint {
		uint digits = 0;
		while (decimal > 0) {
			decimal /= 10;
			digits++;
		}
		return digits;
	};

	// `_ttrl` - traverse tree recurse lambda
	std::function<void(TaxUnit &, int)>
	Display_ttrl = [&Display_ttrl, &DigitCount_l](TaxUnit &unit, int level)
	{
		if (!(unit.is & kind::filter)) level++;

		const size_t shift = !unit.units.empty() && (unit.units[0].is & kind::budget) ? 0 : 1; // only if `budget` present enumeration starts from 0
		const auto pos_width = DigitCount_l(unit.units.size() + shift);

		for (size_t i = 0; i < unit.units.size(); i++) {
			unit.units[i].FormDisplay(level, i+shift, pos_width);
			Display_ttrl(unit.units[i], level);
		}
	};

	assert(parent == nullptr && "Must be a root node");
	this->FormDisplay(0, -1, 0);
	Display_ttrl(*this, 0);
}
