#include "app.hpp"
#include "settings.hpp"

#include "int/imgui/forms/help_marker.hpp"

#include <imgui/imgui.h>

//#include <iostream>

static bool init = true;
static bool treeEditorFlagsAlignLabel = false;

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

	if (ImGui::TreeNodeEx(scast<const void *>(unit), bp::sett.tree.nodeFlags, "%s", scast<const char *>(unit)))
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
	if (bp::sett.tree.showFlagsCheckboxes) {
		im::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow"      , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_OpenOnArrow      );
		im::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnDoubleClick", &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_OpenOnDoubleClick);
		im::CheckboxFlags("ImGuiTreeNodeFlags_SpanAvailWidth"   , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_SpanAvailWidth   ); im::SameLine();	HelpMarker("Extend hit area to all available width instead of allowing more items to be laid out after the node.");
		im::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth"    , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_SpanFullWidth    );
		//im::CheckboxFlags("ImGuiTreeNodeFlags_SpanAllColumns"   , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_SpanAllColumns   ); im::SameLine();	HelpMarker("For use in Tables only.");
		im::CheckboxFlags("ImGuiTreeNodeFlags_Framed"           , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_Framed           );
		im::CheckboxFlags("ImGuiTreeNodeFlags_Selected"         , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_Selected         );
		im::CheckboxFlags("ImGuiTreeNodeFlags_Bullet"           , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_Bullet           );
		im::CheckboxFlags("ImGuiTreeNodeFlags_DefaultOpen"      , &bp::sett.tree.nodeFlags, ImGuiTreeNodeFlags_DefaultOpen      );
		//im::CheckboxFlags("ImGuiTreeNodeFlags_CollapsingHeader", &treeEditorFlags, ImGuiTreeNodeFlags_CollapsingHeader);
		im::Checkbox("Align label with current X position", &treeEditorFlagsAlignLabel);
		//static bool                                        test_drag_and_drop = false;
		//im::Checkbox("Test tree node as drag source"    , &test_drag_and_drop);
	}

	// --------------------------- Data Tree Editor ---------------------------

	// test glyphs
	if (bp::sett.edit.showDebugTextEncoding && im::TreeNode("DebugTextEncoding ##cpp"))
	{
		DebugTextEncodingPrint("Ї");
		DebugTextEncodingPrint("\u0452");
		DebugTextEncodingPrint("\u00C0");
		DebugTextEncodingPrint("\u0403");
		DebugTextEncodingPrint("\u00F7");
		DebugTextEncodingPrint("❤️");
		DebugTextEncodingPrint(std::string("Україна").c_str());

		ImGui::TreePop();
		init = false;
	}

	if (treeEditorFlagsAlignLabel) im::Unindent(im::GetTreeNodeToLabelSpacing());
	im::SetNextItemOpen(true, ImGuiCond_Once);

	if (root)
		TaxUnitEdit(*this->root);

	ImGui::End();
}
