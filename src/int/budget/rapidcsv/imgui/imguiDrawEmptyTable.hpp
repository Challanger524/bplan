#pragma once

#include <imgui.h>

#include <span>
#include <vector>
#include <initializer_list>
#include <assert.h>

namespace UA::budget {


namespace _local {
	constexpr ImGuiTableFlags flagsTable =
	ImGuiTableFlags_NoSavedSettings        |
	ImGuiTableFlags_Reorderable            |
	ImGuiTableFlags_Resizable              |
	ImGuiTableFlags_Hideable               |
	ImGuiTableFlags_ScrollY                |
	ImGuiTableFlags_ScrollX                |
	ImGuiTableFlags_Borders                |
	ImGuiTableFlags_RowBg                  |
	//ImGuiTableFlags_Sortable               |
	//ImGuiTableFlags_SortMulti              |
	//ImGuiTableFlags_SortTristate           |
	ImGuiTableFlags_SizingFixedFit         |
	//ImGuiTableFlags_SizingStretchProp      |
	//ImGuiTableFlags_HighlightHoveredColumn |
	0;

}

/**
 * @param labelS span of c-stringed labels
 * @param labels list of labels to be used for table header creation */
inline void DrawEmptyTable(std::span<const char *const> labelS, const std::initializer_list<size_t> &labels)
{
	if (ImGui::BeginTable("empty table: budget", static_cast<int>(labels.size()), _local::flagsTable))
	{
		//ImGui::TableSetupScrollFreeze(0, 1); // pin header (1st row)
		for (const auto l : labels) {
			assert(l < labelS.size());
			ImGui::TableSetupColumn(labelS[l]);
		}
		ImGui::TableHeadersRow();
		ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeight()); // empty row
		ImGui::EndTable();
	}
}

/**
 * @param labelS span of c-stringed labels
 * @param labelD span of c-stringed descriptions for labels
 * @param labels list of labels to be used for table header creation */
inline void DrawEmptyTable(std::span<const char *const> labelS, std::span<const char *const> labelD, std::initializer_list<size_t> labels)
{
	if (ImGui::BeginTable("empty table: budget", static_cast<int>(labels.size()), _local::flagsTable))
	{
		//ImGui::TableSetupScrollFreeze(0, 1); // pin header (1st row)
		for (const auto l : labels) {
			assert(l < labelS.size());
			assert(l < labelD.size());
			ImGui::TableSetupColumn(labelS[l]);
		}

		// Draw table header row with tooltips
		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		const int columns_count = ImGui::TableGetColumnCount();
		for (int c = 0; c < columns_count; c++)
		{
			if (!ImGui::TableSetColumnIndex(c)) continue;

			const char *const name = ImGui::TableGetColumnName(c);
			ImGui::PushID(c); /*and*/ ImGui::TableHeader(name); /*and*/ ImGui::PopID();

			// tooltip with label description
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", labelD[labels.begin()[c]]);
				ImGui::EndTooltip();
			}
		}

		ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeight()); // empty row
		ImGui::EndTable();
	}
}


} // namespace UA::budget
