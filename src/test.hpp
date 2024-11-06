#pragma once

#include "test/tests.hpp"

#include <imgui.h>

class App;

class Test {
	App &app;

public:
	Test(App *app) : app(*app) {}

	void Menu() {
		if (im::BeginMenu("Test")) {
			im::SeparatorText("oneruns");
			im::Indent();
			{
				im::SeparatorText(".csv");
				if (im::MenuItem("console: income Chernihiv 23 quarter")) { test::ReadCsvQ(); }
				im::Unindent();
			}
			im::SeparatorText("demo");
			im::EndMenu();
		}
	}
};
