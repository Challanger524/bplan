#include "app.hpp"

#include <imgui/imgui.h>

extern bool showImGuiDemoWindow;

void App::MainMenuBar()
{
	if (!im::BeginMainMenuBar()) return;

	// if (im::BeginMenu("File")) { im::EndMenu(); }
	// if (im::BeginMenu("Edit")) { im::EndMenu(); }
	// if (im::BeginMenu("Find")) { im::EndMenu(); }

	/*
	if (im::BeginMenu("View")) {
		//im::SeparatorText("Tree Editor");
		im::EndMenu();
	}
	*/

	// if (im::BeginMenu("Help")) { im::EndMenu(); }

	if (im::BeginMenu("Settings"))
	{
		// im::MenuItem("  GUI", nullptr, false, false); // inactive improvised separator
		im::SeparatorText("GUI");
		im::MenuItem("Dear ImGui: Show Demo Window", nullptr, &showImGuiDemoWindow);
		im::SeparatorText("Debug");
		im::Separator();
		im::EndMenu();
	}

#ifdef TESTING
	this->test.Menu();
#endif

	im::EndMainMenuBar();
}
