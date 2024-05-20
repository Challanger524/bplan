#include "test.hpp"
#include "app/data.hpp"
#include "app.hpp"

#include <imgui.h>

void Test::SkeletonEN()
{
	using kind = tuKind::TaxUnitKind;

	this->app.root.reset(); //? save popup window on erase?
	this->app.root = std::make_unique<TaxUnit>();

	TaxUnit &root = *this->app.root.get();

	root.is   = kind::country;
	root.name = "<Country>";

	root.units = {
		TaxUnit(kind::oblast, "<Region/Oblast>", {
			TaxUnit(kind::rayon_o, "<Rayon>", {
				TaxUnit(kind::filter_tl, "<territorial hromada>", {
					TaxUnit(kind::filter_tl, "<city/village/farm-like>", {
						TaxUnit(kind::hromada, "<Hromada>", {
							TaxUnit(kind::city_capital_whole_oblast, "<City/Village/Farm>", {
								TaxUnit(kind::rayon_c, "<city District (optional)>", {
									TaxUnit(kind::physical, "<physical>", {
										TaxUnit(kind::workers, "<workers>", {
											TaxUnit(kind::worker, "<worker>"),
										}),
										TaxUnit(kind::FOPs, "<FOPs>", {
											TaxUnit(kind::FOP, "<FOP>"),
										})
									}),
									TaxUnit(kind::juridical, "<juridical>", {
										TaxUnit(kind::TOVs, "<TOVs>", {}),
										TaxUnit(kind:: PPs, "<PPs>" , {}),
										TaxUnit(kind:: APs, "<APs>" , {}),
										TaxUnit(kind:: NPs, "<NPs>" , {}),
										TaxUnit(kind::  Gs, "<Gs>"  , {}),
									})
								}),
							}),
						}),
					}),
					TaxUnit(kind::filter_tl, "<village-like>", {}),
					TaxUnit(kind::filter_tl, "<farm-like>", {}),
				})
			}),
		}),
		TaxUnit(kind::oblast, "<Oblast>", {}),
		TaxUnit(kind::oblast, "<Region>", {}),
		TaxUnit(kind::Kyiv, "<Capital>", {}),
	};

	//BREAKPOINT();
	[[maybe_unused]] TaxUnit &itis = root; // for clang vscode debugging
	itis.InitChildsParent();
	itis.InitCapitals();

	itis.Display();
}
