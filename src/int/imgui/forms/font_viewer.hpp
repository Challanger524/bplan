#pragma once

#include"int/imgui/forms/help_marker.hpp"

#include <imgui/imgui.h>

void FontViewer(bool &show)
{
	im::SetNextWindowSize(ImVec2{500, 400}, ImGuiCond_Once);
	if (!im::Begin(__func__, &show))
	{ im::End(); return;}

	// Details for Fonts
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImFontAtlas *atlas = im::GetIO().Fonts;
	if (im::TreeNode("Fonts", "Fonts (%d)", atlas->Fonts.Size))
	{
		for (ImFont *font : atlas->Fonts)
		{
			im::PushID(font);

			bool opened = im::TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
									   font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
			im::SameLine();
			if (im::SmallButton("Set as default"))
				im::GetIO().FontDefault = font;

			if (!opened)
			{ im::PopID(); continue; }

			// Display preview text
			im::PushFont(font);
			im::Text("The quick brown fox jumps over the lazy dog");
			im::PopFont();

			// Display details
			im::SetNextItemWidth(im::GetFontSize() * 8);
			im::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.3f");

			im::SameLine();
			HelpMarker(
				"Note that the default embedded font is NOT meant to be scaled.\n\n"
				"Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
				"You may oversample them to get some flexibility with scaling. "
				"You can also render at multiple sizes and select which one to use at runtime.\n\n"
				"(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");

			im::SameLine();
			if (im::SmallButton("default"))
				font->Scale = 1.f;

			im::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
			for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
				if (font->ConfigData)
					if (const ImFontConfig *cfg = &font->ConfigData[config_i])
						im::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
									   config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH, cfg->GlyphOffset.x, cfg->GlyphOffset.y);

			im::TreePop();
			im::PopID();
		}
		im::TreePop();

	}

	im::End();
}
