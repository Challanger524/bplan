#include "app.hpp"

#include <imgui/imgui.h>

extern bool showImGuiDemoWindow;

void HelpMarker(const char*);

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
	const ImGuiViewport &mainWiewport = *im::GetMainViewport();
	im::SetNextWindowPos (mwWorkArea ? mainWiewport.WorkPos  : mainWiewport.Pos);
	im::SetNextWindowSize(mwWorkArea ? mainWiewport.WorkSize : mainWiewport.Size);

	// - - - - - - - - - - - - Main (viewport) Window - - - - - - - - - - - - -
	if (im::Begin(__func__, nullptr, mwFlags))
	{
		im::Text("_|_|_ Visible with `work area`, swallowed with `main area` (first checkbox below) _|_|_");
		im::Checkbox("Use work area instead of main area", &mwWorkArea);

		im::SameLine();
		HelpMarker(
			"Main Area = entire viewport,\n"
			"Work Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\n"
			"Enable the main-menu bar in Examples menu to see the difference."
			);

		im::CheckboxFlags("ImGuiWindowFlags_NoBackground", &mwFlags, ImGuiWindowFlags_NoBackground);
		im::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &mwFlags, ImGuiWindowFlags_NoDecoration);
		im::Indent();
		im::CheckboxFlags("ImGuiWindowFlags_NoTitleBar"  , &mwFlags, ImGuiWindowFlags_NoTitleBar );
		im::CheckboxFlags("ImGuiWindowFlags_NoCollapse"  , &mwFlags, ImGuiWindowFlags_NoCollapse );
		im::CheckboxFlags("ImGuiWindowFlags_NoScrollbar" , &mwFlags, ImGuiWindowFlags_NoScrollbar);
		im::Unindent();
	}

#if 1 // test main window
	im::NewLine();

	static float f = 0.0f;
	static int counter = 0;
	static ImVec4 clear_color(0.45f, 0.55f, 0.60f, 1.00f);

	im::Text("This is some useful text.");
	im::Checkbox("Demo Window", &showImGuiDemoWindow);
	im::SliderFloat("float", &f, 0.0f, 1.0f);
	im::ColorEdit3("clear color", &clear_color.x);

	if (im::Button("Button")) counter++;
	im::SameLine();
	im::Text("counter = %d", counter);
	if (im::Button("Show Demo Window")) showImGuiDemoWindow = true;

	const ImGuiIO &io = im::GetIO();
	im::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

#endif // test main window

	im::End();
}

void HelpMarker(const char *desc)
{
	im::TextDisabled("(?)");
	if (im::BeginItemTooltip())
	{
		im::PushTextWrapPos(im::GetFontSize() * 35.0f);
		im::TextUnformatted(desc);
		im::PopTextWrapPos();
		im::EndTooltip();
	}
}
