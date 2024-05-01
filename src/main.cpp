#ifndef BPLAN_CONFIG_PCH
#error "./config.hpp" not force included // See "./config.hpp" for details
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <locale.h>
#endif

#include "runconfig.hpp"
#include "app.hpp"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>

int main()
{
#ifdef _WIN32 // console UTF-8
	//system("chcp 65001");
	setlocale(LC_CTYPE, ".UTF8");
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP      (CP_UTF8);
#endif

	SettingsReload();

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

	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.IniFilename = nullptr;
	//io.LogFilename = nullptr;

	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGuiStyle &style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	io.Fonts->AddFontFromFileTTF("res/fonts/FreePixel.ttf"  , 16.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	io.Fonts->AddFontFromFileTTF("res/fonts/ProggyClean.ttf", 13.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

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
	App app;
#if !defined(NDEBUG) || defined(TESTS_IN_APP)
	app.test.DemoCityChernihiv();
#endif
	//------------------------ Main Loop: loop --------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// - - - - - - - - - - main loop: Beginning - - - - - - - - - - - - - -
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
		// - - - - - - - - - - main loop: Executing - - - - - - - - - - - - - -
		app();

		// - - - - - - - - - - main loop: Ending  - - - - - - - - - - - - - - -
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
		glfwWaitEventsTimeout(1.0f); // idling
		glfwPollEvents();
	}

	//------------------------ Deinit: all ------------------------------------
	SettingsUnload();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
