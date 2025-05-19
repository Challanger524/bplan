#ifndef BPLAN_CONFIG_PCH
#error "./config.hpp" not force included // See "./config.hpp" for details
#endif

#include "app.hpp"

#include "int/locale.hpp"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <locale>
#include <chrono>
#include <iostream>
#include <exception>

#include <util/wignore-push.inl>
# if defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__)
#  include <boost/locale.hpp>
# endif
#include <util/wignore-pop.inl>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <locale.h>
#endif // _WIN32

namespace bplan {
#ifndef __clang__
const std::chrono::time_zone *timezone{nullptr};
#endif
}

int main()
{
#ifdef _WIN32 // console UTF-8
	//system("chcp 65001");
	setlocale(LC_CTYPE, ".UTF8");
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP      (CP_UTF8);
#endif

	std::locale locUA; // should be alive for program's lifespan
	try {
#if defined(__MINGW32__) && defined(__GNUC__) && !defined(__clang__) // GCC relies on POSIX locales
		boost::locale::generator loc_gen;                // WinAPI locales:
		locUA =                  loc_gen(bp::LOCALE_UA); //   utf-8 sort - yes, calendar names - no
#else // boost::locale messes things up, like name(): *; and uncaught exceptions on formatting
		locUA = std::locale(bp::LOCALE_UA);
#endif

		std::locale locale = std::locale(locUA, new bp::no_separator(locUA)); // `locale` manages `facet` de-alloc through ref-counter // fix for broken stream output after thousands separator appearance from locale numeric facet

		//ONDEBUG(std::cout << "Trace: Locale(\"\"): " << std::locale("").name() << '\n'); // exception on mingw gcc
		ONDEBUG(std::cout << "Trace: Locale(): "   << std::locale(  ).name() << '\n');
		ONDEBUG(std::cout << "Trace: locale  : "   <<      locale    .name() << '\n');
		std::cout << '\n';

		std::locale::global(locale);
	}
	catch (std::runtime_error &e) { // GCC/libstdc++ uses POSIX locales (not present on Win platform)
		ONDEBUG(std::cerr << "EXCP: " << e.what() << '\n');
		std::cerr << "Warn: locale '" << bp::LOCALE_UA << "' not found, falling back to '" << std::locale().name() << "' locale\n";
		std::cerr << "Warn: some features will not be viewed/work properly (dates, money, text sorting,..)\n";
		std::cerr << "\n";
	}

#ifndef __clang__ // setup UA time zone
	try { bplan::timezone = std::chrono::locate_zone("Europe/Kiev"); }
	catch(const std::runtime_error& e) { std::cerr << "EXCEP: " << e.what() << "\n"; bplan::timezone = nullptr; }
#endif

#if !defined(NDEBUG) && !defined(__clang__)
	try {
		std::cout << "Trace: Timezone name: " << std::chrono::current_zone()->name() << "\n";
		std::cout << "Trace: Sys   time : " << std::chrono::system_clock::now()                                        << "\n";
		std::cout << "Trace: Local time : " << std::chrono::current_zone()->to_local(std::chrono::system_clock::now()) << "\n";
		std::cout << "Trace: Europe/Kiev: " << std::chrono::locate_zone("Europe/Kiev")->to_local(std::chrono::system_clock::now()) << "\n";
		std::cout << '\n';
	}
	catch(const std::runtime_error& e) { std::cerr << "EXCEP: " << e.what() << "\n\n"; }
#endif

	//------------------------ init: GLFW ------------------------------------

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
	glfwSwapInterval(1); // vsync(on)


	//------------------------ init: ImGui -----------------------------------

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


	//------------------------ log: versions ---------------------------------

	std::cout << "Info:  GLFW version: " << glfwGetVersionString() << std::endl;
	std::cout << "Info: ImGui version: " << IMGUI_VERSION;
#ifdef IMGUI_HAS_DOCK
	std::cout << " +docking";
#endif
#ifdef IMGUI_HAS_VIEWPORT
	std::cout << " +viewport";
#endif
	std::cout << std::endl;


	// ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊ ⬋ ⬊
	// ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩ ⭨ ⭩
	// ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩ ⤩
	// ----------------------- program: data ---------------------------------

	App app;

#ifdef TESTING
	app.test.Enable(Test::CSV_GET);
#endif

	//------------------------ main loop: loop -------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// - - - - - - - - - - main loop: starting - - - - - - - - - - - - - -
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// - - - - - - - - - - program: cycling  - - - - - - - - - - - - - - -
		// ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅ ˅

		app();

		// ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄ ˄
		// - - - - - - - - - - program: cycled - - - - - - - - - - - - - - - -


		// - - - - - - - - - - main loop: ending - - - - - - - - - - - - - - -
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


	//------------------------ main: deinit -----------------------------

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
