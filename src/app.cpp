# include "app.hpp"

#include "int/imgui/tools.hpp"
#include "int/imgui/forms.hpp"

#include <imgui/imgui.h>

#include <stdio.h>

//================= class_scope: Public =======================================

void App::operator()(void)
{
	const ImGuiIO &io = ImGui::GetIO();

	// - - - - - - - - - - - app: Apping - - - - - - - - - - - - - -
	this->MainViewportMenuBar();
	//this->MainWindow();

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

	// Dispatch keyboard (shortcuts)
	constexpr bool norepeat = false;
	if (io.KeyCtrl) {
		if (ImGui::IsKeyPressed(ImGuiKey_M, norepeat)) this->showMainViewportMenuBar ^= true;
	}

	{ // render frame stats
		static char framestats[64];
		const auto &framerate = io.Framerate;
		snprintf(framestats, sizeof(framestats), "%5d(%c) %4.fms %2.ffps", ImGui::GetFrameCount(), "|/-\\"[scast<uint>(ImGui::GetTime() * 4) & 3u], 1000.0f / framerate, framerate);
		ImGui::GetForegroundDrawList()->AddText(CalcAlignBottomRight(framestats, ImGui::GetMainViewport()->Size), IM_COL32_WHITE, framestats);
	}

	if (this->show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
}
