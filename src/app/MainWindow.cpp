#include "app.hpp"

#include "int/imgui/forms/help_marker.hpp"

#include <imgui/imgui.h>

void App::MainWindow()
{
	static bool             mwWorkArea = true;   // mw - main window
	static ImGuiWindowFlags mwFlags =            // mw - main window
		ImGuiWindowFlags_NoMove                |
		ImGuiWindowFlags_NoDecoration          |
		ImGuiWindowFlags_NoSavedSettings       |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		//ImGuiWindowFlags_MenuBar             |
		0;

	// Set size / pos
	const ImGuiViewport &mainWiewport = *ImGui::GetMainViewport();
	ImGui::SetNextWindowPos (mwWorkArea ? mainWiewport.WorkPos  : mainWiewport.Pos);
	ImGui::SetNextWindowSize(mwWorkArea ? mainWiewport.WorkSize : mainWiewport.Size);

	if (ImGui::Begin(__func__, nullptr, mwFlags))
	{
		ImGui::Checkbox("Use work area instead of main area", &mwWorkArea);

		ImGui::SameLine();
		HelpMarker(
			"Main Area = entire viewport,\n"
			"Work Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\n"
			"Enable the main-menu bar in Examples menu to see the difference."
			);

		ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &mwFlags, ImGuiWindowFlags_NoBackground);
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &mwFlags, ImGuiWindowFlags_NoDecoration);
		ImGui::Indent();
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar"  , &mwFlags, ImGuiWindowFlags_NoTitleBar );
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse"  , &mwFlags, ImGuiWindowFlags_NoCollapse );
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar" , &mwFlags, ImGuiWindowFlags_NoScrollbar);
		ImGui::Unindent();
	}
	ImGui::End();
}
