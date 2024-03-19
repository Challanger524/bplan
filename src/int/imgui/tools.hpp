#pragma once

#include <imgui/imgui.h>

#include "int/imgui/forms/help_marker.hpp"

// align_text.cpp

ImVec2 CalcAlignTopRight   (const char *text, ImVec2 wcrMax);
ImVec2 CalcAlignBottomRight(const char *text, ImVec2 wcrMax);
void       AlignBottomRight(const char *text, ImVec2 wcrMax);

