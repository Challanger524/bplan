#pragma once

#include "int/imgui/tools.hpp"
#include "int/imgui/forms.hpp"

//#include "int/imgui/forms/help_marker.hpp"
//#include "int/imgui/forms/main_full-window_top-dockspaced.hpp"
#include "int/imgui/forms/main_full-window.hpp"

#include <imgui/imgui.h>

#include <iostream>
#include <stdio.h>

class App
{
bool show_demo_window {true};

public:
	~App() {}
	 App() {}

	App           (      App&&) = delete;
	App           (const App& ) = delete;
	App  operator=(      App&&) = delete;
	App& operator=(const App& ) = delete;

	void operator()(void);
};

inline void App::operator()(void)
{
	const ImGuiIO io = ImGui::GetIO();


#ifdef MAIN_WINDOW_DOCKSPACED
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		//const ImGuiID docspMainID = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		 const ImGuiID dockspaceMainViewportId = MainFullWindowTopDockspaced();
		// const ImGuiID dockspaceMainViewportId = MainTopfullWindowDockspaced();
	}
#elif 1
	MainFullWindow();
#endif

	if (ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		static float f = 0.0f;
		static int counter = 0;
		static ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Text("This is some useful text.");
		ImGui::Checkbox("Demo Window", &show_demo_window);
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", &clear_color.x);

		if (ImGui::Button("Button"))
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		if (ImGui::Button("Show Demo Window"))
			show_demo_window = true;

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	} ImGui::End();

	{ // render frame stats
		static char framestats[64];
		snprintf(framestats, sizeof(framestats), "%5d(%c) %4.fms %2.ffps", ImGui::GetFrameCount(), "|/-\\"[uint(ImGui::GetTime() * 3) & 3], 1000.0f / io.Framerate, io.Framerate);
		ImGui::GetForegroundDrawList()->AddText(CalcAlignBottomRight(framestats, ImGui::GetMainViewport()->WorkSize), IM_COL32_WHITE, framestats);
	}

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
}

