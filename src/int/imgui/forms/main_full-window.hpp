#include "int/imgui/tools.hpp"

#include <imgui/imgui.h>

#ifndef MAIN_WINDOW_NOT_DOCKSPACED
#define MAIN_WINDOW_NOT_DOCKSPACED
#endif

inline void MainFullWindow()
{
	static bool use_work_area = true;
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;

	// We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
	// Based on your use case you may want one or the other.
	const ImGuiViewport *mainWiewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos (use_work_area ? mainWiewport->WorkPos  : mainWiewport->Pos);
	ImGui::SetNextWindowSize(use_work_area ? mainWiewport->WorkSize : mainWiewport->Size);

	if (ImGui::Begin("Fullscreen window", nullptr, flags))
	{
		ImGui::Checkbox("Use work area instead of main area", &use_work_area);

		ImGui::SameLine();
		HelpMarker(
			"Main Area = entire viewport,\n"
			"Work Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\n"
			"Enable the main-menu bar in Examples menu to see the difference."
			);

		ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &flags, ImGuiWindowFlags_NoBackground);
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &flags, ImGuiWindowFlags_NoDecoration);
		ImGui::Indent();
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar"  , &flags, ImGuiWindowFlags_NoTitleBar );
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse"  , &flags, ImGuiWindowFlags_NoCollapse );
		ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar" , &flags, ImGuiWindowFlags_NoScrollbar);
		ImGui::Unindent();
	} ImGui::End();
}
