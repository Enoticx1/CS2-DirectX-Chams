//============ Copyright KiwiHax, All rights reserved ============//
//
//  Purpose: 
//
//================================================================//

#pragma once

#include <vector>
#include "../imgui.h"

#define IMADD_ANIMATIONS_SPEED	0.07f

struct ImGuiWindow;

namespace ImAdd
{
	// Helpers
	ImVec4  HexToColorVec4(unsigned int hex_color, float alpha = 1.0f);
	float	GetColorPickerWidth();
	
	// Separators
	void    SeparatorText(const char* label, float thickness = 1.0f);
	void    VSeparator(float margin = 0.0f, float thickness = 1.0f);

	// Widgets
	bool	SelectableLabel(const char* label, bool selected, bool centered = false, const ImVec2& size_arg = ImVec2(0, 0));
	void    ApplyViperStyle();
	bool    CheckBox(const char* label, bool* v);
	bool	Button(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags button_flags = 0);
	bool	ButtonAccent(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags button_flags = 0);
	bool	Combo(const char* label, int* selected_index, std::vector<const char*> items);
	bool	ColorEdit4(const char* label, float col[4]);
	bool	KeyBind(const char* str_id, ImGuiKey* k, const ImVec2& size_arg = ImVec2(0, 0));

	// Child Windows
	bool	Tab(const char* label, bool selected, const ImVec2& size_arg = ImVec2(0, 0));
	void	ScrollBar(const char* str_id, ImGuiWindow* window, const ImVec2& size_arg = ImVec2(0, 0));
	bool	BeginChild(const char* str_id, std::vector<const char*> tabs, int* selected_tab_index_callback, const ImVec2& size_arg = ImVec2(0, 0));
	bool	BeginChild(const char* str_id, std::vector<const char*> tabs, const ImVec2& size_arg = ImVec2(0, 0));
	bool	BeginChild(const char* str_id, const ImVec2& size_arg = ImVec2(0, 0));
	void    EndChild();

	// Sliders
	bool	SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL);
	bool	SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f");
	bool	SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");

	// Drawing
	void	RenderText(ImVec2 pos, const char* text, const char* text_end = NULL, bool hide_text_after_hash = true, bool has_outlines = false);
}
