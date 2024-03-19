// Algining text tools, see declarations in: "int/imgui/tools.hpp"

#include <imgui/imgui.h>

constexpr float rightPadd = 4.0;

ImVec2 CalcAlignTopRight   (const char *text, const ImVec2 wcrMax)
{
	const float textSizeX = ImGui::CalcTextSize(text).x;

	ImVec2 textIdent(wcrMax.x - textSizeX, 0);
		   textIdent.x -= rightPadd; // to not cut by the border
	return textIdent;
}

ImVec2 CalcAlignBottomRight(const char *text, const ImVec2 wcrMax)
{
	const ImVec2 textSize = ImGui::CalcTextSize(text);

	ImVec2 textIdent = wcrMax - textSize;
		   textIdent.x -= rightPadd; // to not cut by the border
	return textIdent;
}

void   AlignBottomRight    (const char *text, const ImVec2 wcrMax)
{
	const ImVec2 savedCursorPos = ImGui::GetCursorPos();

	const ImVec2 textPos = CalcAlignBottomRight(text, wcrMax);
	ImGui::SetCursorPos(textPos);
	ImGui::Text("%s", text);

	ImGui::SetCursorPos(savedCursorPos);
}
