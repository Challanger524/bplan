#include "test.hpp"
#include "app/data.hpp"
#include "app.hpp"

#include <imgui.h>

void Test::DemoCityChernihiv()
{
	using kind = tuKind::TaxUnitKind;

	this->app.root.reset();
	this->app.root = std::make_unique<TaxUnit>();

	TaxUnit &tuRoot = *this->app.root.get();

	tuRoot.is   = kind::country;
	tuRoot.name = "Україна";

	tuRoot.units = {
		TaxUnit(kind::oblast, "Чернігівська область", {
			TaxUnit(kind::rayon_o, "Чернігівський район", {
				TaxUnit(kind::filter_tl, "територіальна громада", {
					TaxUnit(kind::filter_tl, "міська", {
						TaxUnit(kind::hromada, "Чернігівська МТГ", {
							TaxUnit(kind::city_capital_whole_oblast, "м.Чернігів", {
								TaxUnit(kind::rayon_c, "Новозаводський район", {
									TaxUnit(kind::physical, "фізичні особи", {
										TaxUnit(kind::workers, "наймані робітники", {
											TaxUnit(kind::worker, "<найманий робітник>"),
										}),
										TaxUnit(kind::FOPs, "ФОПи", {
											TaxUnit(kind::FOP, "<ФОП>"),
										})
									}),
									TaxUnit(kind::juridical, "юридичні особи", {
										TaxUnit(kind::TOVs, "ТОВ", {}),
										TaxUnit(kind:: PPs, "ПП" , {}),
										TaxUnit(kind:: APs, "АП" , {}),
										TaxUnit(kind:: NPs, "НП" , {}),
										TaxUnit(kind::  Gs, "Г"  , {}),
									})
								}),
								TaxUnit(kind::rayon_c, "Деснянський район", {}),
							}),
						}),
					}),
					TaxUnit(kind::filter_tl, "селищна", {}),
					TaxUnit(kind::filter_tl, "сільска", {}),
				})
			})
		}),
		TaxUnit(kind::oblast, "Сумська область", {}),
		TaxUnit(kind::Kyiv, "м.Київ", {}),
		//TaxUnit(kind::Simferopol, "м.Сімферополь", {}),
		//TaxUnit(kind::region, "Автономна Республіка Крим", {}),
	};

	auto &fops = // FOP
		tuRoot.   // 1 - country
		units[0]. // 2 - oblast Che
		units[0]. // 3 - rayon  Che
		units[0]. // 4 - hromada
		units[0]. // 5 - miska
		units[0]. // 6 - miska hromada Che
		units[0]. // 7 - city Che
		units[0]. // 8 - city rayon
		units[0]. // 9 - physical
		units[1]; // 10- FOP

	fops.units.resize(100'000); // 10
	for (auto &u : fops.units) u.is = fops.units[0].is;

	auto &che_h = tuRoot.units[0].units[0].units[0].units[0].units[0]; // Чернігівська міська громада
	che_h.budget = std::make_unique<Budget>();
	che_h.budget->bcode = 2555900000;
	che_h.budget->budgetY.resize(1);
	che_h.budget->budgetY[0].year = std::chrono::year(2024);
	che_h.budget->budgetY[0].incomesQ.resize(4);
	che_h.budget->budgetY[0].incomesQ[0].incomesM.resize(3);
	che_h.budget->budgetY[0].incomesQ[0].incomesM[0].table.resize(4);
	//che_h.budget->budgetY[0].incomesQ[0].Load();
	//exit(-1);

	//BREAKPOINT();
	[[maybe_unused]] TaxUnit &itis = tuRoot; // for clang vscode debugging
	itis.InitChildsParent();
	itis.InitCapitals();

	itis.Display();
}
