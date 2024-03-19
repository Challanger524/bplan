#ifndef BPLAN_CONFIG_PCH
#error "./config.hpp" not force included // See "./config.hpp" for details
#endif

#include "int/imgui/forms/main_full-window_top-dockspaced.hpp"
#include "int/imgui/forms/main_topfull_window_dockspaced.hpp"

#include "int/imgui/tools.hpp"
#include "int/imgui/forms.hpp"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>
#include <stdio.h>

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
	//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   // Not supported by ImGui yet - will make ImGui to disappear
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Means loosing control of the window position. Maybe can be done through connecting ImGui and GLFW move callbacks

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

#ifdef MAIN_WINDOW_TITLE_FPS // View fps on glfw window title. unwelcomed due to significant bump in events
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

#ifdef MAIN_WINDOW_DOCKSPACED
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			const ImGuiID docspMainID = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			//const ImGuiID dockspaceMainViewportId = MainFullWindowTopDockspaced();
			//const ImGuiID dockspaceMainViewportId = MainTopfullWindowDockspaced();
		}
#endif

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		//ImGui::SetNextWindowDockID(dockspaceMainViewportId, ImGuiCond_Once);
		if (ImGui::Begin("Window"))
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

	return 0;
}
