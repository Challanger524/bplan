#include "app.hpp"

#include "int/imgui/forms/help_marker.hpp"

#include <imgui/imgui.h>

//#include <iostream>

static bool init = true;
static bool treeEditorFlagsAlignLabel = false;
static ImGuiTreeNodeFlags treeEditorFlags =
	ImGuiTreeNodeFlags_OpenOnArrow       |
	ImGuiTreeNodeFlags_OpenOnDoubleClick |
	ImGuiTreeNodeFlags_SpanAllColumns    |
	ImGuiTreeNodeFlags_SpanAvailWidth    |
	//ImGuiTreeNodeFlags_SpanFullWidth   |
	ImGuiTreeNodeFlags_Framed            |
	//ImGuiTreeNodeFlags_Selected        |
	//ImGuiTreeNodeFlags_Bullet          |
	//ImGuiTreeNodeFlags_CollapsingHeader        | // TreePush()
	0;

#include <iostream>
void DebugTextEncodingPrint(const char *s) {
	im::DebugTextEncoding(s);
	if (init) std::cout << s << std::endl;
}

void TaxUnitEdit(TaxUnit &unit, short level = -1)
{
	if (level != -1){ level++; }

	// Expand topmost filter (coz it has only 3 elements)
	if (unit.is & tuKind::filter_tl && !unit.units.empty() && unit.units[0].is == tuKind::filter_tl) {
		im::SetNextItemOpen(true, ImGuiCond_Once);
	}

	const auto& treeTag = unit.display.empty() ? unit.name.c_str() : unit.display.c_str();
	//if (ImGui::TreeNodeEx(scast<const void *>(unit), treeEditorFlags, "%s", scast<const char *>(unit)))
	if (ImGui::TreeNodeEx(scast<const void *>(unit), treeEditorFlags, "%s", scast<const char *>(unit)))
	{
		for (auto &tu : unit.units)
			TaxUnitEdit(tu, level);

		im::PushID(scast<const void *>(unit));
#if 0 // test data storage/processing
		if (ImGui::SmallButton("+")) {
			unit.units.emplace_back(TaxUnit(unit.units.empty() ? TaxUnit::kind::none : unit.units[0].is, "<+>"));
			unit.InitChildsParent();
		}
#endif
		im::PopID();

		ImGui::TreePop();
	}

}

void App::DataTreeEditor()
{
	// --------------------------- Window -------------------------------------
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
	const ImGuiViewport *viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos (viewport->WorkPos );
	ImGui::SetNextWindowSize(viewport->WorkSize);

	if (!ImGui::Begin(__func__, nullptr, flags))
	{ ImGui::End(); return; }

	// --------------------------- Data Tree Editor: setup --------------------

	im::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow"      , &treeEditorFlags, ImGuiTreeNodeFlags_OpenOnArrow      );
	im::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnDoubleClick", &treeEditorFlags, ImGuiTreeNodeFlags_OpenOnDoubleClick);
	im::CheckboxFlags("ImGuiTreeNodeFlags_SpanAvailWidth"   , &treeEditorFlags, ImGuiTreeNodeFlags_SpanAvailWidth   );
	im::SameLine();	HelpMarker("Extend hit area to all available width instead of allowing more items to be laid out after the node.");
	im::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth"    , &treeEditorFlags, ImGuiTreeNodeFlags_SpanFullWidth    );
	im::CheckboxFlags("ImGuiTreeNodeFlags_SpanAllColumns"   , &treeEditorFlags, ImGuiTreeNodeFlags_SpanAllColumns   );
	im::SameLine();	HelpMarker("For use in Tables only.");
	im::CheckboxFlags("ImGuiTreeNodeFlags_Framed"           , &treeEditorFlags, ImGuiTreeNodeFlags_Framed           );
	im::CheckboxFlags("ImGuiTreeNodeFlags_Selected"         , &treeEditorFlags, ImGuiTreeNodeFlags_Selected         );
	im::CheckboxFlags("ImGuiTreeNodeFlags_Bullet"           , &treeEditorFlags, ImGuiTreeNodeFlags_Bullet           );
	im::CheckboxFlags("ImGuiTreeNodeFlags_DefaultOpen"      , &treeEditorFlags, ImGuiTreeNodeFlags_DefaultOpen      );
	//im::CheckboxFlags("ImGuiTreeNodeFlags_CollapsingHeader", &treeEditorFlags, ImGuiTreeNodeFlags_CollapsingHeader);
	im::Checkbox("Align label with current X position", &treeEditorFlagsAlignLabel);
	//static bool                                        test_drag_and_drop = false;
	//im::Checkbox("Test tree node as drag source"    , &test_drag_and_drop);

	// --------------------------- Data Tree Editor ---------------------------

#if 1 // test charset
	if(im::TreeNode("DebugTextEncoding ##cpp"))
	{
		DebugTextEncodingPrint("ђ");
		DebugTextEncodingPrint("\u0452");
		DebugTextEncodingPrint("\u00C0");
		DebugTextEncodingPrint("\u0403");
		DebugTextEncodingPrint("\u00F7");
#ifndef NO_CHAR8_T // with c++20 (silly) utf support
		DebugTextEncodingPrint(reinterpret_cast<const char*>(std::u32string(U"\U00000423\U0000043a\U00000440\U00000430\U00000457\U0000043d\U00000430").c_str()));
		DebugTextEncodingPrint(reinterpret_cast<const char*>(std::u16string(u"\u0423\u043a\u0440\u0430\u0457\u043d\u0430").c_str()));
		DebugTextEncodingPrint(reinterpret_cast<const char*>(std:: u8string(u8"Україна").c_str()));
#elif 0
		DebugTextEncodingPrint(                              std::   string(u8"Україна").c_str());
#endif
		DebugTextEncodingPrint(                              std::   string(  "Україна").c_str());
		ImGui::TreePop();
		init = false;
	}
#endif

	if (treeEditorFlagsAlignLabel) im::Unindent(im::GetTreeNodeToLabelSpacing());
	im::SetNextItemOpen(true, ImGuiCond_Once);
	TaxUnitEdit(this->root);

	ImGui::End();
}
