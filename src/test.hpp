#pragma once

//#include "app.hpp"

#include <imgui.h>

class App;

class Test {
	App &app;

public:
	Test(App *app) : app(*app) {}

	void DrawMenuBar() {
		if (im::BeginMenu("Test")) {
			im::SeparatorText("skeleton");
			if (im::MenuItem("Skeleton [en]")) { this->SkeletonEN(); }
			if (im::MenuItem("Skeleton [ua]")) { this->SkeletonUA(); }
			im::SeparatorText("demo");
			if (im::MenuItem("Chernihiv city demo")) { this->DemoCityChernihiv(); }
			im::EndMenu();
		}
	}

	void SkeletonEN();
	void SkeletonUA();
	void DemoCityChernihiv();
};
