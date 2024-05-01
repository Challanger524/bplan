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

	tuRoot.
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

	//BREAKPOINT();
	[[maybe_unused]] TaxUnit &itis = tuRoot; // for clang vscode debugging
	itis.InitChildsParent();
	itis.InitCapitals();

	itis.SwitchDisplayLevel();
}
