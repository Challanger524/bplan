#pragma once

#include <imgui/imgui.h>

namespace bplan {

struct Settings { // global program settings
	// bool <flag> : 1 = 1; // no `&addr` operator allowed

	struct Application {

		//bool showMainViewportMenuBar = 1;
		bool showImGuiDemoWindow     = 0;

		struct Editor {
			bool showDebugTextEncoding = 0;

			struct Tree {
				bool showFlagsCheckboxes = 0; // tree node flags

				ImGuiTreeNodeFlags nodeFlags =               // tree-node flags
					ImGuiTreeNodeFlags_OpenOnArrow       |
					ImGuiTreeNodeFlags_OpenOnDoubleClick |
					ImGuiTreeNodeFlags_SpanAllColumns    |
					ImGuiTreeNodeFlags_SpanAvailWidth    |
					ImGuiTreeNodeFlags_Framed            |
					//ImGuiTreeNodeFlags_Bullet            |
					//ImGuiTreeNodeFlags_Selected          |
					//ImGuiTreeNodeFlags_SpanFullWidth     |
					//ImGuiTreeNodeFlags_CollapsingHeader  | // TreePush()
					0;

				struct Display {
					bool depthLevel            = 0;
					//bool nodeEnumer            = 0; // tree node enumeration of same-level
				} display, &disp = display;
			} tree;
		} editor, &edit = editor;
	} application, &app = application;

	using App  = Application;
	using Edit = App  ::Editor ; Edit &edit = app.editor; // shortcut
	using Tree = Edit ::Tree   ; Tree &tree = edit.tree ; // shortcut
	using Disp = Tree ::Display; Disp &disp = tree.disp ; // shortcut

} inline settings, &sett = settings;

#if 0
inline auto &app   = sett.app ; // `alias` to global (run) configuration variable
inline auto &edit  = app .edit; // shortcut
inline auto &tree  = edit.tree; // shortcut
inline auto &disp  = tree.disp; // shortcut
#endif

}
