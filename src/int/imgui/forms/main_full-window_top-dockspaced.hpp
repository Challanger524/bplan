#pragma once

#include "int/imgui/tools.hpp"
#include <imgui/imgui.h>
#include <stdio.h>

#ifndef MAIN_WINDOW_DOCKSPACED
#define MAIN_WINDOW_DOCKSPACED
#endif

inline
ImGuiID MainFullWindowTopDockspaced()
{
	/*return*/ ImGuiID dockspaceID{};
	const      ImGuiIO io = ImGui::GetIO();
	const ImGuiViewport *mainViewport = ImGui::GetMainViewport();

	static bool opt_fullscreen = true;
	static bool opt_padding    = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGui::SetNextWindowViewport(mainViewport->ID);
		ImGui::SetNextWindowPos(mainViewport->WorkPos);
		ImGui::SetNextWindowSize(mainViewport->WorkSize); //! set custom size
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		//window_flags ^= ImGuiWindowFlags_MenuBar; // remove tofull window menu bar
	}
	else
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	//------------------------ Topfull imgui window (inside glfw window): begin

	ImGui::Begin("MainFilledWindow", nullptr, window_flags);

	if (!opt_padding)	ImGui::PopStyleVar(1);
	if (opt_fullscreen)	ImGui::PopStyleVar(2);

	//------------------------ Custom Dockspace (inside Topfull window)
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		//! move size constraining from Dockspace to Topfull Window
		// calc and apply size for the next new workspace
		IM_ASSERT(ImGui::GetWindowContentRegionMin() == ImGui::GetCursorPos());

		const ImVec2 wcrMin = ImGui::GetWindowContentRegionMin(); // `wcr` - Window Content Region
		const ImVec2 wcrMax = ImGui::GetWindowContentRegionMax(); // `wcr` - Window Content Region
		ImGui::GetForegroundDrawList()->AddRect(wcrMin, wcrMax, IM_COL32(255, 255, 0, 255)); // yellow - work region of full(screen) window

		const ImVec2 wcrDockspaceNoText(wcrMax.x, wcrMax.y - ImGui::GetTextLineHeight()); // dockspace region size (full  **minus text height**)
		ImGui::GetForegroundDrawList()->AddRect(wcrMin, wcrDockspaceNoText, IM_COL32_WHITE); // white

		const ImVec2 wcrDockspaceNotextSize = wcrDockspaceNoText - wcrMin; // dockspace region size (full  **minus text height**)

		dockspaceID = ImGui::GetID("MainFullWindowTopDockspaced");
		ImGui::DockSpace(dockspaceID, wcrDockspaceNotextSize, dockspace_flags);
	}

	//------------------------ Menu Bar of the Topfull Window
	if (ImGui::BeginMenuBar()) // menu definition order does not matter
	{
		if (ImGui::BeginMenu("Options"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
			ImGui::MenuItem("Padding"   , NULL, &opt_padding   );
			ImGui::Separator();

			if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
			if (ImGui::MenuItem("Flag: NoDockingSplit"          , "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit          ) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
			if (ImGui::MenuItem("Flag: NoUndocking"             , "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking             ) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking;
			if (ImGui::MenuItem("Flag: NoResize"                , "", (dockspace_flags & ImGuiDockNodeFlags_NoResize                ) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
			if (ImGui::MenuItem("Flag: AutoHideTabBar"          , "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar          ) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			if (ImGui::MenuItem("Flag: PassthruCentralNode"     , "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode     ) != 0, opt_fullscreen)) dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;

			ImGui::Separator();
			ImGui::EndMenu();
		}

		HelpMarker(
			"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!\n"
			"- Drag from window title bar or their tab to dock/undock.\n"
			"- Drag from window menu button (upper-left button) to undock an entire node (all windows).\n"
			"- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)\n"
			"- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)\n"
			"This demo app has nothing to do with enabling docking!\n\n"
			"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window.\n\n"
			"Read comments in ShowExampleAppDockSpace() for more details."
			);

			ImGui::EndMenuBar();
	}

	ImGui::End(); // <- # ImGui::Begin("MainFilledWindow", nullptr, window_flags);

	return dockspaceID;
}
