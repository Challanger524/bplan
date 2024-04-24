#pragma once

#include <imgui/imgui.h>

namespace bplan::settings { // global program settings

struct Application {
	// bool <flag> : 1 = 1; // no `&addr` allowed

	//bool showMainViewportMenuBar = 1;
	bool showImGuiDemoWindow     = 0;

	struct Editor {
		bool showDebugTextEncoding = 0;

		struct Tree {
			bool showFlagsCheckboxes = 0; // tree node flags

			ImGuiTreeNodeFlags tnFlags =               // tree-node flags
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
				bool level            = 0;
				//bool numer            = 0; // enumeration of same-level tree-nodes
			} display, &disp = display;
		} tree;
	} editor, &edit = editor;
} /*inline application*/;

inline Application     application;     //            global (run) configuration variable
inline auto &app     = application;     // `alias` to global (run) configuration variable
inline auto &edit    = app.editor;      // shortcut
inline auto &tree    = edit.tree;       // shortcut
inline auto &disp    = tree.display;    // shortcut

}

namespace bplan {
	namespace sett = settings; // `namespace alias` to global program settings
}
