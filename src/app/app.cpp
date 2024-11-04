#include "app.hpp"

#include <imgui/imgui.h>

#include <stdio.h>

bool showImGuiDemoWindow = false;

ImVec2 CalcAlignBottomRight(const char*, ImVec2);

void App::operator()(void)
{
	const ImGuiIO &io = im::GetIO();

	// - - - - - - - - - - - app: Apping - - - - - - - - - - - - - -
	this->MainMenuBar();
	this->MainWindow();

	// Dispatch keyboard (shortcuts)
	if (io.KeyCtrl) {
		//if (im::IsKeyPressed(ImGuiKey_M, false)) bp::sett.showMainViewportMenuBar ^= true;
	}

	{ // render frame stats
		static char framestats[64];
		snprintf(framestats, sizeof(framestats), "%5d(%c) %4.fms %2.ffps", im::GetFrameCount(), "|/-\\"[scast<uint>(im::GetTime() * 4) & 3u], 1000.0f / io.Framerate, io.Framerate);
		im::GetForegroundDrawList()->AddText(CalcAlignBottomRight(framestats, im::GetMainViewport()->Size), IM_COL32_WHITE, framestats);
	}

	if (showImGuiDemoWindow) im::ShowDemoWindow(&showImGuiDemoWindow);
}

constexpr float rightPadd = 4.0; // padding from boarder

ImVec2 CalcAlignBottomRight(const char *text, const ImVec2 wcrMax)
{
	const ImVec2 textSize = im::CalcTextSize(text);

	ImVec2 textIdent = wcrMax - textSize;
		   textIdent.x -= rightPadd; // to not leave text cut by the border
	return textIdent;
}
