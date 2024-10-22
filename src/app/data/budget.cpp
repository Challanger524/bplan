#include "budget.hpp"

#include "parse/csv.hpp"
#include "imgui.h"

#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <format>

//using std::chrono::year;

void Budget::Yearof::IncomesQ::Load()
{
	//csv::Print("Downloads/test.csv");
	csv::Print("Downloads/local_2555900000_INCOMES_2023_quarter.csv");
}

void Budget::Edit()
{
	static const ImGuiTabBarFlags tab_bar_flags = ImGuiTabItemFlags_NoTooltip|ImGuiTabItemFlags_NoReorder;

	im::Text("Year:"); im::SameLine();
	if (ImGui::BeginTabBar("Year_of", tab_bar_flags))
	{
		//if (ImGui::TabItemButton("Year:", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip)) ImGui::OpenPopup("MyHelpMenu");

		for (const auto &year : this->budgetY) { // iterate years
			if (ImGui::BeginTabItem(std::to_string(scast<int>(year.year)).c_str()))
			{
				im::Text("Type:"); im::SameLine(); //! -> checkboxes
				if (ImGui::BeginTabBar("Type_of", tab_bar_flags))
				{
					if (ImGui::BeginTabItem("combined")) { ImGui::EndTabItem(); }
					if (!year.incomesQ.empty() && ImGui::BeginTabItem("incomes"))
					{
						im::Text("Quarter:"); im::SameLine();
						if (ImGui::BeginTabBar("Quarter_of", tab_bar_flags)) // quarters
						{
							if (ImGui::BeginTabItem("year")) { ImGui::EndTabItem(); }
							//for (const auto &quart : year.incomesQ) {
							for (size_t i = 0; i < year.incomesQ.size(); i++) { // iterate quarter
								const auto &quart = year.incomesQ[i];
								if (im::BeginTabItem(std::format("Q{}", i).c_str()))
								{
									im::Text("Month:"); im::SameLine();
									if (ImGui::BeginTabBar("Month_of", tab_bar_flags)) // quarters
									{
										if (ImGui::BeginTabItem("quarter")) { ImGui::EndTabItem(); }
										for (size_t j = 0; j < quart.incomesM.size(); j++) // iterate quarter
										{
											const auto &month = quart.incomesM[j];
											if (im::BeginTabItem(std::format("{:02}", i*3+j).c_str()))
											{
												// month table
												ImGui::EndTabItem();
											}
										}
										ImGui::EndTabBar();
									}
									ImGui::EndTabItem();
								}
							}
							ImGui::EndTabBar();
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
#if 0
	static const ImGuiTreeNodeFlags subtreeFlags =
		ImGuiTreeNodeFlags_OpenOnArrow       |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAllColumns    |
		ImGuiTreeNodeFlags_SpanAvailWidth    |
		//ImGuiTreeNodeFlags_Framed            |
		//ImGuiTreeNodeFlags_Bullet            |
		//ImGuiTreeNodeFlags_Selected          |
		//ImGuiTreeNodeFlags_SpanFullWidth     |
		//ImGuiTreeNodeFlags_CollapsingHeader  | // TreePush()
		0;

	im::Unindent();
	if (ImGui::TreeNodeEx("##subtree", subtreeFlags))
	{

		im::TreePop();
	}
#endif
}
