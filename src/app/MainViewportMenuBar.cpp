#include "app.hpp"
#include "settings.hpp"

#include <imgui/imgui.h>

void App::MainViewportMenuBar()
{
	//if (bp::sett.showMainViewportMenuBar && im::BeginMainMenuBar())
	if (im::BeginMainMenuBar())
	{
		//if (im::BeginMenu("File")) { im::EndMenu(); }
		//if (im::BeginMenu("Edit")) { im::EndMenu(); }
		//if (im::BeginMenu("Find")) { im::EndMenu(); }

		if (im::BeginMenu("View")) {
			im::SeparatorText("Tree Editor");
			if (im::MenuItem("Display Level", nullptr, sett::disp.level)) { root.SwitchDisplayLevel(); }
			im::EndMenu();
		}

		//if (im::BeginMenu("Help")) { im::EndMenu(); }

		if (im::BeginMenu("Settings")) {
			//im::MenuItem("  GUI", nullptr, false, false);
			im::SeparatorText("GUI");
			//im::MenuItem("Main menu bar"     , "Ctrl+M", &bp::sett.showMainViewportMenuBar);
			im::MenuItem("Dear ImGui: Show Demo Window",  nullptr, &sett::app.showImGuiDemoWindow);
			im::SeparatorText("Debug");
			im::MenuItem("Show Tree Flags", nullptr, &sett::tree.showFlagsCheckboxes);
			im::MenuItem("Show Debug Text Encoding", nullptr, &sett::edit.showDebugTextEncoding);
			im::Separator();
			im::EndMenu();
		}

		im::EndMainMenuBar();
	}
}
