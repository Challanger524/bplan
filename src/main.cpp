#ifndef BPLAN_CONFIG_PCH
#error `config.hpp` not force included // Check "./config.hpp" for details
#endif

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>
#include <stdio.h>

ImGuiID DockspaceMainViewport();
ImVec2 TextBottomRight(const char *text);

int main()
{
	//------------------------ Init: GLFW -------------------------------------
	// Init/Setup GLFW (window, contexts, OS messages processing)
	glfwSetErrorCallback([](int, const char *description) noexcept { std::cerr << "Error: " << description << std::endl; });

	if (!glfwInit())
	{ std::cerr << "Error: glfwInit() fail\n"; return -1; }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_DEBUG        , GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);                // 3.0+
	glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE); // 3.2+
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   //Not supported by ImGui yet - will make ImGui to disappear
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); //Means loosing control of the window position. Maybe can be done through connecting ImGui and GLFW move callbacks

	GLFWwindow *const window = glfwCreateWindow(1280, 720, "bplan", nullptr, nullptr);
	if (!window)
	{ glfwTerminate(); return -1; }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	//------------------------ Init: Dear ImGui -------------------------------
	// Init/Setup ImGui (graphical user interface for C++)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (ImGuiStyle &style = ImGui::GetStyle(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	//------------------------ Log: first: prints -----------------------------
	// Print used dependencies versions
	std::cout << "Info:  GLFW version: " << glfwGetVersionString() << std::endl;
	std::cout << "Info: ImGui version: " << IMGUI_VERSION;
#ifdef IMGUI_HAS_DOCK
	std::cout << " +docking";
#endif
#ifdef IMGUI_HAS_VIEWPORT
	std::cout << " +viewport";
#endif
	std::cout << std::endl;

	//------------------------ Main Loop: data --------------------------------
	bool show_demo_window = true;
	//------------------------ Main Loop: loop --------------------------------
	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

#ifdef MAIN_WINDOW_TITLE_FPS // View fps on glfw window title. unwelcomed due to significant amount bump in events
		{
			static int frames = 0;
			static double t_last = glfwGetTime();
			const  double t_curr = glfwGetTime();
			static char animaTitleMain[64];
			if (t_curr - t_last > 1.0 || frames == 0)
			{
				double fps = double(frames) / (t_curr - t_last);
				sprintf(animaTitleMain, "bplan FPS: %.1f", fps);
				glfwSetWindowTitle(window, animaTitleMain);
				t_last = t_curr;
				frames = 0;
			}
			frames++;
			glfwSetWindowTitle(window, animaTitleMain);
		}
#endif

		//const ImGuiID docspMainID = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		const ImGuiID dockspaceMainViewportId = DockspaceMainViewport();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::SetNextWindowDockID(dockspaceMainViewportId, ImGuiCond_Once);
		if (ImGui::Begin("Main Window"))
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

		// main: loop: Finalize
		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(window);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		//static unsigned short frame_extra = 0;
		//glfwWaitEvents();
		//if (++frame_extra > 2) { frame_extra = 0; glfwWaitEvents(); }
		glfwWaitEventsTimeout(1.0f);
		//if (++frame_extra > 2) { frame_extra = 0; glfwWaitEventsTimeout(1.0f); }
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

ImVec2 CalcTextBottomRightPos(const char * text) {
	const ImVec2 wcrMax = ImGui::GetWindowContentRegionMax();
	const ImVec2 textSize = ImGui::CalcTextSize(text);

	return ImVec2(wcrMax.x - textSize.x - 4, wcrMax.y - textSize.y);
}

ImVec2 TextBottomRight(const char * text) {
	const auto savedCursorPos = ImGui::GetCursorPos();

	const ImVec2 textPos = CalcTextBottomRightPos(text);
	ImGui::SetCursorPos(textPos);
	ImGui::Text("%s", text);

	ImGui::SetCursorPos(savedCursorPos);
	return textPos;
}

ImGuiID DockspaceMainViewport()
{
	/*return*/ ImGuiID dockspace_id{};
	const ImGuiIO io = ImGui::GetIO();

	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
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

	// Create `full(glfw)screen window`
	ImGui::Begin("MainWindowEmbedded", nullptr, window_flags);

	if (!opt_padding)
		ImGui::PopStyleVar();
	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	static char metrics[64];
	snprintf(metrics, sizeof(metrics), "%5d(%c) %4.fms %2.ffps", ImGui::GetFrameCount(), "|/-\\"[uint(ImGui::GetTime() * 3) & 3], 1000.0f / io.Framerate, io.Framerate);
	ImGui::GetForegroundDrawList()->AddText(CalcTextBottomRightPos(metrics), IM_COL32(255, 255, 255, 255), metrics);
	// TextBottomRight(metrics); // overlappable text

	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		IM_ASSERT(ImGui::GetWindowContentRegionMin() == ImGui::GetCursorPos());

		// `wcr` - Window Content Region
		const ImVec2 wcrMin = ImGui::GetWindowContentRegionMin();
		const ImVec2 wcrMax = ImGui::GetWindowContentRegionMax();
		ImGui::GetForegroundDrawList()->AddRect(wcrMin, wcrMax, IM_COL32(255, 255, 0, 255)); // yellow - work region of full(screen) window

		const ImVec2 wcrDockspaceNoText(wcrMax.x, wcrMax.y - ImGui::GetTextLineHeight()); // dockspace region size (full  **minus text height**)
		ImGui::GetForegroundDrawList()->AddRect(wcrMin, wcrDockspaceNoText, IM_COL32(255, 255, 255, 255)); // white

		const ImVec2 wcrDockspaceNoTextSize = wcrDockspaceNoText - wcrMin; // dockspace region size (full  **minus text height**)

		dockspace_id = ImGui::GetID("Dockspace-Main");
		ImGui::DockSpace(dockspace_id, wcrDockspaceNoTextSize, dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
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

		const char helpMarkerDescription[] =
			"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!\n"
			"- Drag from window title bar or their tab to dock/undock.\n"
			"- Drag from window menu button (upper-left button) to undock an entire node (all windows).\n"
			"- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)\n"
			"- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)\n"
			"This demo app has nothing to do with enabling docking!\n\n"
			"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window.\n\n"
			"Read comments in ShowExampleAppDockSpace() for more details.";

		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(helpMarkerDescription);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End(); // "DockSpace Demo"

	return dockspace_id;
}
