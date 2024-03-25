#include "app.hpp"

#include <imgui/imgui.h>

void App::MainViewportMenuBar()
{
	if (this->showMainViewportMenuBar && im::BeginMainMenuBar())
	{
		//if (im::BeginMenu("File")) { im::EndMenu(); }
		//if (im::BeginMenu("Edit")) { im::EndMenu(); }
		//if (im::BeginMenu("Find")) { im::EndMenu(); }
		//if (im::BeginMenu("View")) { im::EndMenu(); }
		//if (im::BeginMenu("Help")) { im::EndMenu(); }

		if (im::BeginMenu("Settings")) {
			im::MenuItem("  GUI", nullptr, false, false);
			im::MenuItem("Main menu bar", "Ctrl+M", &this->showMainViewportMenuBar);
			im::MenuItem("Dear Imgui: demo window", nullptr, &this->show_demo_window);
			im::Separator();
			im::EndMenu();
		}

		im::EndMainMenuBar();
	}
}
