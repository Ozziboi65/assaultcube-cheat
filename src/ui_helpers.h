#pragma once
#include "ect/imgui/imgui.h"
#include <cstring>


inline void TextGradient(const char* text, ImVec4 startColor, ImVec4 endColor) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    const char* textEnd = text + strlen(text);
    ImVec2 textSize = ImGui::CalcTextSize(text, textEnd);
    
    // Draw each character with interpolated color
    float x = pos.x;
    for (const char* c = text; c < textEnd; c++) {
        if (*c == '\0') break;
        

        float progress = (c - text) / (float)(textEnd - text);
        

        ImVec4 color = ImVec4(
            startColor.x + (endColor.x - startColor.x) * progress,
            startColor.y + (endColor.y - startColor.y) * progress,
            startColor.z + (endColor.z - startColor.z) * progress,
            startColor.w + (endColor.w - startColor.w) * progress
        );
        
        // Draw single character
        char buf[2] = { *c, '\0' };
        ImVec2 charSize = ImGui::CalcTextSize(buf);
        drawList->AddText(ImVec2(x, pos.y), ImGui::ColorConvertFloat4ToU32(color), buf);
        x += charSize.x;
    }
    
    // Move cursor to after the text
    ImGui::Dummy(textSize);
}


namespace GradientPresets {
    // Rainbow colors
    inline void Rainbow(const char* text) {
        TextGradient(text, 
            ImVec4(1.0f, 0.0f, 0.0f, 1.0f),  // Red
            ImVec4(0.5f, 0.0f, 1.0f, 1.0f)); // Purple
    }
    
    // Cyan to pink
    inline void CyanPink(const char* text) {
        TextGradient(text, 
            ImVec4(0.0f, 1.0f, 1.0f, 1.0f),  // Cyan
            ImVec4(1.0f, 0.0f, 1.0f, 1.0f)); // Pink
    }
    
    // Gold to orange
    inline void GoldOrange(const char* text) {
        TextGradient(text, 
            ImVec4(1.0f, 0.84f, 0.0f, 1.0f), // Gold
            ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
    }
    
    inline void Sexy(const char* text) {
        TextGradient(text, 
            ImVec4(0.40f, 0.85f, 1.00f, 1.00f), // quite sigma id say
            ImVec4(0.05f, 0.40f, 0.90f, 1.00f) 
        );
    }
    // Blue to green
    inline void BlueGreen(const char* text) {
        TextGradient(text, 
            ImVec4(0.0f, 0.5f, 1.0f, 1.0f),  // Blue
            ImVec4(0.0f, 1.0f, 0.5f, 1.0f)); // Green
    }
}
