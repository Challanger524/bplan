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
			//if (im::MenuItem("Display Level", nullptr, bp::sett.disp.depthLevel)) { root->Display(); }
			if (im::MenuItem("Display Level"               , nullptr, &bp::sett.disp.depthLevel)) { root->Display(); }
			if (im::MenuItem("Display Enumeration: node"   , nullptr, &bp::sett.disp.nodeEnumer)) { root->Display(); }
			if (im::MenuItem("Display Enumeration: element", nullptr, &bp::sett.disp.elemEnumer)) { root->Display(); }
			im::EndMenu();
		}

		//if (im::BeginMenu("Help")) { im::EndMenu(); }

		if (im::BeginMenu("Settings")) {
			//im::MenuItem("  GUI", nullptr, false, false);
			im::SeparatorText("GUI");
			//im::MenuItem("Main menu bar"     , "Ctrl+M", &bpgit ::sett.showMainViewportMenuBar);
			im::MenuItem("Dear ImGui: Show Demo Window",  nullptr, &bp::sett.app.showImGuiDemoWindow);
			im::SeparatorText("Debug");
			im::MenuItem("Show Tree Flags", nullptr, &bp::sett.tree.showFlagsCheckboxes);
			im::MenuItem("Show Debug Text Encoding", nullptr, &bp::sett.edit.showDebugTextEncoding);
			im::Separator();
			im::EndMenu();
		}

		#if !defined(NDEBUG) || defined(TESTS_IN_APP)
			this->test.DrawMenuBar();
		#endif

		im::EndMainMenuBar();
	}
}
