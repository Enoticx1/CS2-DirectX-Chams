//============ Copyright KiwiHax, All rights reserved ============//
//
//  Purpose: 
//
//================================================================//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui_internal.h"
#include "imgui_addons.h"

#include <map>
#include <string>

using namespace ImGui;

ImVec4 ImAdd::HexToColorVec4(unsigned int hex_color, float alpha)
{
    ImVec4 color;

    color.x = ((hex_color >> 16) & 0xFF) / 255.0f;
    color.y = ((hex_color >> 8) & 0xFF) / 255.0f;
    color.z = (hex_color & 0xFF) / 255.0f;
    color.w = alpha;

    return color;
}

float ImAdd::GetColorPickerWidth()
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    return g.FontSize * 2.0f + style.CellPadding.x * 4.0f;
}

void ImAdd::SeparatorText(const char* label, float thickness)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(-0.1f, g.FontSize), label_size.x, g.FontSize);

    const ImRect total_bb(pos, pos + size);
    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id)) {
        return;
    }

    window->DrawList->AddText(pos, GetColorU32(ImGuiCol_TextDisabled), label);

    if (thickness > 0)
        window->DrawList->AddLine(pos + ImVec2(label_size.x + style.ItemInnerSpacing.x, size.y / 2), pos + ImVec2(size.x, size.y / 2), GetColorU32(ImGuiCol_Border), thickness);
}

void ImAdd::VSeparator(float margin, float thickness)
{
    if (thickness <= 0)
        return;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(thickness, -0.1f), thickness, thickness);

    const ImRect bb(pos, pos + size);
    const ImRect bb_rect(pos + ImVec2(0, margin), pos + size - ImVec2(0, margin));

    ItemSize(ImVec2(thickness, 0.0f));
    if (!ItemAdd(bb, 0))
        return;

    window->DrawList->AddRectFilled(bb_rect.Min, bb_rect.Max, GetColorU32(ImGuiCol_Border));
}

bool ImAdd::SelectableLabel(const char* label, bool selected, bool centered, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    // Colors
    ImVec4 colLabel = GetStyleColorVec4(hovered || selected ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    ImVec4 colLineMain = GetStyleColorVec4(ImGuiCol_SliderGrab);
    ImVec4 colLineNull = colLineMain;
    colLineNull.w = 0.0f;

    ImVec4 colLine = selected ? colLineMain : colLineNull;

    // Animations
    struct stColors_State {
        ImColor Label;
        ImColor Line;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Label = colLabel;
        it_anim->second.Line = colLine;
    }

    it_anim->second.Label.Value = ImLerp(it_anim->second.Label.Value, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);
    it_anim->second.Line.Value = ImLerp(it_anim->second.Line.Value, colLine, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    //RenderNavCursor(total_bb, id);

    window->DrawList->AddText(pos + ImTrunc(ImVec2(centered ? (size.x / 2 - label_size.x / 2) : 0.0f, size.y / 2 - label_size.y / 2)), it_anim->second.Label, label);

    return pressed;
}

static float ease_out_quad(float x) {
    return 1.0f - (1.0f - x) * (1.0f - x);
}

static ImU32 lerp_color(ImU32 a, ImU32 b, float t) {
    ImVec4 va = ColorConvertU32ToFloat4(a);
    ImVec4 vb = ColorConvertU32ToFloat4(b);
    return ColorConvertFloat4ToU32(ImLerp(va, vb, t));
}

bool ImAdd::CheckBox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = 10.0f;
    const float padding = 8.0f;
    const ImVec2 pos = ImVec2(window->DC.CursorPos.x + padding, window->DC.CursorPos.y + 3);
    const float row_height = ImMax(square_sz, label_size.y) + 6;
    
    const ImRect total_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y), 
                          ImVec2(window->DC.CursorPos.x + GetContentRegionAvail().x, window->DC.CursorPos.y + row_height));
    ItemSize(total_bb, 0);
    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    static std::map<ImGuiID, float> anim_map;
    static std::map<ImGuiID, float> hover_map;
    
    if (anim_map.find(id) == anim_map.end()) anim_map[id] = *v ? 1.0f : 0.0f;
    if (hover_map.find(id) == hover_map.end()) hover_map[id] = 0.0f;

    anim_map[id] = ImLerp(anim_map[id], *v ? 1.0f : 0.0f, 0.20f);
    hover_map[id] = ImLerp(hover_map[id], hovered ? 1.0f : 0.0f, 0.20f);

    float anim = anim_map[id];
    float hover_anim = hover_map[id];

    const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));
    
    ImU32 border_col = lerp_color(IM_COL32(55, 55, 60, 255), GetColorU32(ImGuiCol_Header), hover_anim);
    window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, IM_COL32(35, 35, 40, 255), 0.0f);
    window->DrawList->AddRect(check_bb.Min, check_bb.Max, border_col, 0.0f, 0, 1.0f);
    
    if (anim > 0.01f)
    {
        float inner_pad = 2.0f;
        float fill_size = (square_sz - inner_pad * 2) * ease_out_quad(anim);
        float offset = (square_sz - fill_size) * 0.5f;
        ImVec2 fill_min = ImVec2(check_bb.Min.x + offset, check_bb.Min.y + offset);
        ImVec2 fill_max = ImVec2(fill_min.x + fill_size, fill_min.y + fill_size);
        
        ImVec4 accent_v4 = style.Colors[ImGuiCol_Header];
        ImU32 fill_col = GetColorU32(ImVec4(accent_v4.x, accent_v4.y, accent_v4.z, anim));
        window->DrawList->AddRectFilled(fill_min, fill_max, fill_col, 0.0f);
    }
    
    ImVec2 label_pos = ImVec2(check_bb.Max.x + 6, pos.y + (square_sz - label_size.y) * 0.5f);
    ImU32 text_col = lerp_color(IM_COL32(140, 140, 145, 255), IM_COL32(220, 220, 225, 255), anim);
    window->DrawList->AddText(label_pos, text_col, label);

    return pressed;
}

bool ImAdd::Button(const char* label, const ImVec2& size_arg, ImDrawFlags draw_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    // Colors
    ImU32 colFrame = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

    //RenderNavCursor(total_bb, id);
    RenderFrame(total_bb.Min, total_bb.Max, colFrame, true, style.FrameRounding);
	window->DrawList->AddRect(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
	RenderText(pos + ImTrunc((size - label_size) / 2), label);

    return pressed;
}

bool ImAdd::ButtonAccent(const char* label, const ImVec2& size_arg, ImDrawFlags draw_flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_Header]);
    PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_HeaderHovered]);
    PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_HeaderActive]);
    PushStyleColor(ImGuiCol_BorderShadow, style.Colors[ImGuiCol_BorderShadow]);

    bool result = Button(label, size_arg, draw_flags);

	PopStyleColor(4);

    return result;
}

bool ImAdd::Combo(const char* label, int* selected_index, std::vector<const char*> items)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float square_sz = g.FontSize + style.FramePadding.y * 2.0f;

    const float width = CalcItemWidth();
    const float height = GetFrameHeight();

    int items_count = items.size();

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(width, height);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    std::string popup_str_id = std::string(std::string(label) + "::combo_popup");

    if (pressed)
    {
        OpenPopup(popup_str_id.c_str());
    }

    PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.FramePadding.y));
    if (BeginPopupEx(GetID(popup_str_id.c_str()), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {
        SetWindowPos(pos + ImVec2(0, height + style.FramePadding.y), ImGuiCond_Always);
        SetWindowSize(ImVec2(width, ImGui::GetFontSize() * items_count + style.FramePadding.y * (items_count + 1)), ImGuiCond_Always);

        for (int i = 0; i < items_count; i++)
        {
            if (ImAdd::SelectableLabel(items[i], i == *selected_index, false, ImVec2(GetContentRegionAvail().x, GetFontSize())))
            {
                *selected_index = i;
                CloseCurrentPopup();
            }
        }

        EndPopup();
    }
    PopStyleVar(2);

    // Colors
    ImVec4 colFrame = GetStyleColorVec4((hovered && held) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    // Animations
    struct stColors_State {
        ImColor Frame;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
    }

    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    //RenderNavCursor(total_bb, id);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, it_anim->second.Frame, style.FrameRounding);
    window->DrawList->AddRect(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    std::string preview_item;
    if (*selected_index > items.size()) {
        preview_item = "*unknown item*";
    }
    else
    {
        preview_item = items[*selected_index];
    }

    window->DrawList->AddText(pos + style.FramePadding, GetColorU32(ImGuiCol_Text), preview_item.c_str());

    RenderArrow(window->DrawList, pos + ImVec2(width - GetFontSize() - style.FramePadding.x, style.FramePadding.y), GetColorU32(ImGuiCol_Text), ImGuiDir_Down);

    return pressed;
}

bool ImAdd::ColorEdit4(const char* label, float col[4])
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImVec4 col_v4(col[0], col[1], col[2], col[3]);
	const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip;

    bool pressed = ColorButton(label, col_v4, flags, ImVec2(g.FontSize * 2 + style.CellPadding.x * 4.0f, g.FontSize + style.CellPadding.y * 2.0f - style.FrameBorderSize));

    if (pressed)
    {
		OpenPopup(label);
    }

    if (BeginPopup(label))
    {
        ColorPicker4(label, col, flags);
        EndPopup();
    }

    return pressed;
}

bool ImAdd::KeyBind(const char* str_id, ImGuiKey* k, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);

    ImVec2 pos = window->DC.CursorPos;

    char buf_display[32] = "Unbinded";

    bool is_selecing = false;

    //if (*k != 0 && g.ActiveId != id)
    //{
    //    strcpy_s(buf_display, sizeof buf_display, GetKeyName(*k));
    //}
    //else 
    if (g.ActiveId == id)
    {
        is_selecing = true;
        strcpy_s(buf_display, sizeof buf_display, "...");
    }

    ImVec2 size = CalcItemSize(size_arg, 100, g.FontSize) + style.CellPadding * 2.0f;
    ImRect frame_bb(pos, pos + size);
    ImRect total_bb(pos, frame_bb.Max);

    ImGui::ItemSize(total_bb);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);

    // Colors
    ImVec4 colLabel = GetStyleColorVec4(is_selecing ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImColor Label;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Label = colLabel;
    }

    it_anim->second.Label.Value = ImLerp(it_anim->second.Label.Value, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    const bool user_clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false);

    if (user_clicked)
    {
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false))
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    //int key = *k;

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, false))
    {
        if (g.ActiveId != id)
        {
            // Start capturing
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            SetActiveID(id, window);
            FocusWindow(window);
        }
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false) && g.ActiveId == id && !hovered)
    {
        // Clicked outside - cancel
        ClearActiveID();
    }

    // Handle key capture
    if (g.ActiveId == id)
    {
        // Check keyboard keys if no mouse button was pressed
        if (!value_changed)
        {
            // Check all possible keys
            for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) // only named keyboard/gamepad keys
            {
                ImGuiKey key_test = (ImGuiKey)i;

                // Skip mouse inputs
                if ((key_test >= ImGuiKey_MouseLeft && key_test <= ImGuiKey_MouseWheelY) || key_test == ImGuiKey_Escape)
                    continue;

                if (IsKeyPressed(key_test)) // Pressed, not Down, avoids "instant bind"
                {
                    *k = key_test;
                    value_changed = true;
                    ClearActiveID();
                    break;
                }
            }
        }

        // Escape cancels
        if (IsKeyPressed(ImGuiKey_Escape))
        {
            ClearActiveID();
        }
    }

    // Render

    //ImGui::RenderNavHighlight(total_bb, id);

    window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(ImGuiCol_FrameBg), style.FrameRounding);

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    ImVec2 buf_display_size = ImGui::CalcTextSize(buf_display, NULL, true);
    window->DrawList->AddText(pos + style.CellPadding, it_anim->second.Label, buf_display);

    return value_changed;
}

bool ImAdd::Tab(const char* label, bool selected, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Colors
    const ImU32 col = GetColorU32(selected ? ImGuiCol_ChildBg : hovered && held ? ImGuiCol_TabActive : hovered ? ImGuiCol_TabHovered : ImGuiCol_Tab);

    // Render
    //RenderNavCursor(bb, id);
    RenderFrame(bb.Min, bb.Max, col, false);

    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max,
        GetColorU32(selected ? ImGuiCol_BorderShadow : ImGuiCol_BorderShadow), GetColorU32(selected ? ImGuiCol_BorderShadow : ImGuiCol_BorderShadow),
        GetColorU32(selected ? ImGuiCol_BorderShadow : ImGuiCol_BorderShadow), GetColorU32(selected ? ImGuiCol_BorderShadow : ImGuiCol_BorderShadow)
    );

    if (!selected)
    {
        window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y - style.ChildBorderSize), ImVec2(bb.Max.x, bb.Max.y - style.ChildBorderSize), GetColorU32(ImGuiCol_Border), style.ChildBorderSize);
    }

    RenderText(pos + ImTrunc((size - label_size) / 2) - ImVec2(0.0f, style.ChildBorderSize), label);

    return pressed;
}

void ImAdd::ScrollBar(const char* str_id, ImGuiWindow* window, const ImVec2& size_arg)
{
    if (!window || window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, GetFrameHeight(), CalcItemWidth());
    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    // Scroll metrics
    float visible_height = size.y;
    float total_height = window->ContentSize.y;
    float scroll_max = ImMax(window->ScrollMax.y, 0.0f);
    float scroll_y = window->Scroll.y;

    float scroll_height = (total_height > 0.0f)
        ? (visible_height / total_height) * visible_height
        : visible_height;

    scroll_height = ImClamp(scroll_height, 15.0f, visible_height);

    float scroll_top = (scroll_max > 0.0f)
        ? (scroll_y / scroll_max) * (visible_height - scroll_height)
        : 0.0f;

    // Handle drag-to-scroll
    if (held && scroll_max > 0.0f)
    {
        float mouse_delta = g.IO.MouseDelta.y;
        float scrollable_range = visible_height - scroll_height;
        if (scrollable_range > 0.0f)
        {
            float ratio = scroll_max / scrollable_range;
            window->Scroll.y = ImClamp(window->Scroll.y + mouse_delta * ratio, 0.0f, scroll_max);
        }
    }

	// Handle mouse wheel scrolling
    bool hovered_window = IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    if (!held && (hovered || hovered_window) && scroll_max > 0.0f && !IsKeyDown(ImGuiKey_LeftShift))
    {
        // Disable native ImGui scrolling
        window->Flags |= ImGuiWindowFlags_NoScrollWithMouse;

        const float wheel_speed = 40.0f; // tweak to taste
        window->Scroll.y = ImClamp(
            window->Scroll.y - g.IO.MouseWheel * wheel_speed,
            0.0f,
            scroll_max
        );
    }

    // Colors
    ImU32 colGrab = GetColorU32((hovered && held) ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);

    // Draw background and grab
    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_ScrollbarBg), style.ScrollbarRounding);
    window->DrawList->AddRectFilled(
        ImVec2(total_bb.Min.x, total_bb.Min.y + scroll_top),
        ImVec2(total_bb.Max.x, total_bb.Min.y + scroll_top + scroll_height),
        colGrab,
        style.ScrollbarRounding
    );

    //RenderNavCursor(total_bb, id);
}

bool ImAdd::BeginChild(const char* str_id, std::vector<const char*> tabs, int* selected_tab_index_callback, const ImVec2& size_arg)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    //if (parent_window->SkipItems)
    //    return;

    const ImGuiID id = parent_window->GetID(str_id);
    const ImGuiStyle& style = g.Style;

	std::string str_id_tabs         = std::string(str_id) + "##child##tabs";
	std::string str_id_scrollbar    = std::string(str_id) + "##child##scrollbar";

    PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding);
	bool result = ImGui::BeginChild(str_id, size_arg, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    PopStyleVar();

    //if (result)
    {
        ImGuiWindow*    window  = GetCurrentWindow();
        ImVec2          cur_pos = window->DC.CursorPos;
        ImVec2          pos     = window->Pos;
        ImVec2          size    = window->Size;
		ImRect		    window_bb(pos, pos + size);
        bool            has_scroll = window->ScrollMax.y > 0;

        bool has_tabs = tabs.size() > 0;
        float tabs_height = GetFrameHeight();

        if (has_tabs)
        {
            SetCursorScreenPos(pos + ImVec2(0.0f, style.ChildBorderSize * 4.0f));
            PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.ChildBorderSize, 0.0f));
            if (ImGui::BeginChild(str_id_tabs.c_str(), ImVec2(size.x, tabs_height), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground))
            {
                PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));
                PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

                float tab_width = ImTrunc((size.x - style.ChildBorderSize * (tabs.size() + 1)) / tabs.size());

                for (int i = 0; i < tabs.size(); i++)
                {
                    bool selected = false;

                    static std::map<ImGuiID, int> anim;
                    auto it_anim = anim.find(id);

                    if (it_anim == anim.end())
                    {
                        anim.insert({ id, int() });
                        it_anim = anim.find(id);

                        it_anim->second = 0;
                    }

                    if (selected_tab_index_callback)
                    {
						selected = (*selected_tab_index_callback == i);
                    }
                    else
                    {
						selected = (it_anim->second == i);
                    }

                    if (Tab(tabs[i], selected, ImVec2(i == (tabs.size() - 1) ? GetContentRegionAvail().x : tab_width, tabs_height)))
                    {
                        if (selected_tab_index_callback)
                        {
							*selected_tab_index_callback = i;
                        }
                        else
                        {
                            it_anim->second = i;
                        }
                    }

                    if (i < tabs.size() - 1)
                    {
                        SameLine();
						VSeparator(0.0f, style.ChildBorderSize);
                        SameLine();
                    }
                }

                PopStyleVar(2);
            }
            ImGui::EndChild();
            PopStyleVar();
        }

        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
        {
			window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(ImGuiCol_ChildBg));

            if (style.ChildBorderSize > 0.0f)
            {
                // Window outer border
                window->DrawList->AddRect(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.ChildBorderSize);

                // Window top decoration
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.ChildBorderSize, style.ChildBorderSize), ImVec2(window_bb.Max.x - style.ChildBorderSize, window_bb.Min.y + style.ChildBorderSize), ImGui::GetColorU32(ImGuiCol_Header), style.ChildBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.ChildBorderSize, style.ChildBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.ChildBorderSize, window_bb.Min.y + style.ChildBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.ChildBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.ChildBorderSize, style.ChildBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.ChildBorderSize, window_bb.Min.y + style.ChildBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_Border), style.ChildBorderSize);
            }
        }

		float scroll_offset_y = style.ChildBorderSize * 3.0f + (has_tabs ? tabs_height : style.ChildBorderSize);

        if (has_scroll)
        {
            SetCursorScreenPos(pos + ImVec2(size.x - style.ScrollbarSize - style.WindowPadding.x, scroll_offset_y + style.WindowPadding.y));
            ImAdd::ScrollBar(str_id_scrollbar.c_str(), window, ImVec2(style.ScrollbarSize, size.y - scroll_offset_y - style.WindowPadding.y * 2.0f));
        }

        SetCursorScreenPos(cur_pos + ImVec2(0.0f, style.ChildBorderSize * 3.0f + (has_tabs ? tabs_height : 0.0f)));

        if (has_scroll)
        {
            window->ContentRegionRect.Max.x -= style.ScrollbarSize + style.WindowPadding.x;
        }

        window->DrawList->PushClipRect(window_bb.Min + ImVec2(style.ChildBorderSize, style.ChildBorderSize * 3.0f + (has_tabs ? tabs_height : style.ChildBorderSize)), window_bb.Max - ImVec2(style.ChildBorderSize, has_tabs ? 0.0f : style.ChildBorderSize), true);
    }

    PushItemWidth(GetContentRegionAvail().x);

    return result;
}

bool ImAdd::BeginChild(const char* str_id, std::vector<const char*> tabs, const ImVec2& size_arg)
{
	return ImAdd::BeginChild(str_id, tabs, (int*)NULL, size_arg);
}

bool ImAdd::BeginChild(const char* str_id, const ImVec2& size_arg)
{
	return ImAdd::BeginChild(str_id, {}, (int*)NULL, size_arg);
}

void ImAdd::EndChild()
{
    PopItemWidth();

    ImGuiWindow* window = GetCurrentWindow();

    window->DrawList->PopClipRect();

    ImGui::EndChild();
}

bool ImAdd::SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float width = CalcItemWidth();
    const ImVec2 pos = window->DC.CursorPos;

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const bool has_label = label_size.x > 0;
    const float frame_pos_y = has_label ? (g.FontSize + style.ItemInnerSpacing.y) : 0.0f;
    const float frame_height = g.FontSize;

    const ImRect frame_bb(pos + ImVec2(0, frame_pos_y), pos + ImVec2(width, frame_pos_y + frame_height));
    const ImRect total_bb(pos, frame_bb.Max);

    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id, &frame_bb, 0))
        return false;

    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(frame_bb, id, 0);
    const bool clicked = hovered && ImGui::IsMouseClicked(0, (ImGuiInputFlags)0, id);
    const bool held = g.ActiveId == id;
    const bool make_active = (clicked || g.NavActivateId == id);

    if (make_active)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Colors
    ImU32 colFrame = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImU32 colLine = GetColorU32(held ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);

    // Grab logic
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, 0, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    float relative_value = 0.0f;
    if (data_type == ImGuiDataType_Float)
    {
        float val = *(float*)p_data;
        float min_val = *(float*)p_min;
        float max_val = *(float*)p_max;
        relative_value = (val - min_val) / (max_val - min_val);
    }
    else if (data_type == ImGuiDataType_S32)
    {
        int val = *(int*)p_data;
        int min_val = *(int*)p_min;
        int max_val = *(int*)p_max;
        relative_value = (float)(val - min_val) / (float)(max_val - min_val);
    }

    relative_value = ImClamp(relative_value, 0.0f, 1.0f);

    // Draw frame

    const float pad = ImTrunc(frame_height / 3.0f);

    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, colFrame, style.FrameRounding);

    ImVec2 fill_end = ImTrunc(ImVec2(frame_bb.Min.x + relative_value * frame_bb.GetWidth(), frame_bb.Max.y));

    ImRect slider_bb = ImRect(ImTrunc(frame_bb.Min), ImTrunc(fill_end));

    if (slider_bb.Max.x > slider_bb.Min.x + style.FrameRounding)
    {
        window->DrawList->AddRectFilled(slider_bb.Min, slider_bb.Max, colLine, style.FrameRounding);
    }

    window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    
    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

    if (has_label) {
        RenderText(total_bb.Min, label);
		PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
        RenderText(total_bb.Min + ImVec2(width - CalcTextSize(value_buf).x, 0), value_buf);
        PopStyleColor();
    }

    return value_changed;
}

bool ImAdd::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format);
}

bool ImAdd::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

void ImAdd::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash, bool has_outlines)
{
    ImGuiContext& g = *GImGui;

    if (has_outlines)
    {
        PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_Border]);

        ImGui::RenderText(pos + ImVec2(0, 1), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(0, -1), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(1, 0), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(-1, 0), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(1, 1), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(-1, -1), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(1, -1), text, text_end, hide_text_after_hash);
        ImGui::RenderText(pos + ImVec2(-1, 1), text, text_end, hide_text_after_hash);

        PopStyleColor();
    }

    ImGui::RenderText(pos, text, text_end, hide_text_after_hash);
}

void ImAdd::ApplyViperStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 0;
    style.ChildRounding     = 0;
    style.FrameRounding     = 0;
    style.PopupRounding     = 0;
    style.GrabRounding      = 0;
    style.ScrollbarRounding = 0;

    style.WindowBorderSize  = 1;
    style.FrameBorderSize   = 1;
    style.PopupBorderSize   = 1;

    style.WindowPadding     = ImVec2(9, 9);
    style.FramePadding      = ImVec2(5, 4);
    style.CellPadding       = ImVec2(2, 2);
    style.ItemSpacing       = ImVec2(7, 7);
    style.ItemInnerSpacing  = ImVec2(5, 6);
    style.WindowMinSize     = ImVec2(0, 0);

    style.ScrollbarSize     = 6.0f;

    style.Colors[ImGuiCol_WindowBg]             = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_ChildBg]              = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_PopupBg]              = ImAdd::HexToColorVec4(0x181818);

    style.Colors[ImGuiCol_Text]                 = ImAdd::HexToColorVec4(0xffffff);
    style.Colors[ImGuiCol_TextDisabled]         = ImAdd::HexToColorVec4(0x848484);

    style.Colors[ImGuiCol_Border]               = ImAdd::HexToColorVec4(0x000000);
    style.Colors[ImGuiCol_Separator]            = style.Colors[ImGuiCol_Border];

    style.Colors[ImGuiCol_Header]               = ImAdd::HexToColorVec4(0x00A36C); // Viper Green
    style.Colors[ImGuiCol_HeaderHovered]        = ImAdd::HexToColorVec4(0x00C38C);
    style.Colors[ImGuiCol_HeaderActive]         = ImAdd::HexToColorVec4(0x00834C);

    style.Colors[ImGuiCol_SliderGrab]           = style.Colors[ImGuiCol_Header];
    style.Colors[ImGuiCol_SliderGrabActive]     = style.Colors[ImGuiCol_HeaderActive];

    style.Colors[ImGuiCol_Button]               = ImAdd::HexToColorVec4(0x232323);
    style.Colors[ImGuiCol_ButtonHovered]        = ImAdd::HexToColorVec4(0x252525);
    style.Colors[ImGuiCol_ButtonActive]         = ImAdd::HexToColorVec4(0x212121);

    style.Colors[ImGuiCol_FrameBg]              = ImAdd::HexToColorVec4(0x232323);
    style.Colors[ImGuiCol_FrameBgHovered]       = ImAdd::HexToColorVec4(0x252525);
    style.Colors[ImGuiCol_FrameBgActive]        = ImAdd::HexToColorVec4(0x212121);

    style.Colors[ImGuiCol_Tab]                  = ImAdd::HexToColorVec4(0x1b1b1b);
    style.Colors[ImGuiCol_TabHovered]           = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_TabActive]            = ImAdd::HexToColorVec4(0x1a1a1a);

    style.Colors[ImGuiCol_BorderShadow]         = ImAdd::HexToColorVec4(0x000000, 0.5f);
}
