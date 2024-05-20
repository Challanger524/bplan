#include "test.hpp"
#include "app/data.hpp"
#include "app.hpp"

#include <imgui.h>

void Test::SkeletonUA()
{
	using kind = tuKind::TaxUnitKind;

	this->app.root.reset();
	this->app.root = std::make_unique<TaxUnit>();

	TaxUnit &tuRoot = *this->app.root.get();

	tuRoot.is   = kind::country;
	tuRoot.name = "<Країна>";

	tuRoot.units = {
		TaxUnit(kind::oblast, "<Регіон/Область>", {
			TaxUnit(kind::rayon_o, "<Район>", {
				TaxUnit(kind::filter_tl, "<територіальна громада>", {
					TaxUnit(kind::filter_tl, "<міська/селищна/сільська>", {
						TaxUnit(kind::hromada, "<Громада>", {
							TaxUnit(kind::city_capital_whole_oblast, "<Місто/Селище/Село>", {
								TaxUnit(kind::rayon_c, "<Район міста (опціонально)>", {
									TaxUnit(kind::physical, "<фізичні>", {
										TaxUnit(kind::workers, "<наймані працівники>", {
											TaxUnit(kind::worker, "<найманий працівник>"),
										}),
										TaxUnit(kind::FOPs, "<ФОПи>", {
											TaxUnit(kind::FOP, "<ФОП>"),
										}),
									}),
									TaxUnit(kind::juridical, "<юридичні особи>", {
										TaxUnit(kind::TOVs, "<ТОВ>", {}),
										TaxUnit(kind:: PPs, "<ПП>" , {}),
										TaxUnit(kind:: APs, "<АП>" , {}),
										TaxUnit(kind:: NPs, "<НП>" , {}),
										TaxUnit(kind::  Gs, "<Г>"  , {}),
									}),
								}),
							}),
						}),
					}),
					TaxUnit(kind::filter_tl, "<селищна>", {}),
					TaxUnit(kind::filter_tl, "<сільска>", {})
				}),
			}),
		}),
		TaxUnit(kind::oblast, "<Область>", {}),
		TaxUnit(kind::Kyiv, "<Столиця>", {}),
		//TaxUnit(kind::Simferopol, "м.Сімферополь", {}),
		//TaxUnit(kind::region, "Автономна Республіка Крим", {}),
	};

	//BREAKPOINT();
	[[maybe_unused]] TaxUnit &itis = tuRoot; // for clang vscode debugging
	itis.InitChildsParent();
	itis.InitCapitals();

	itis.Display();
}
