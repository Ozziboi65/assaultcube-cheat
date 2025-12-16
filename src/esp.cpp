#include "esp.h"
#include "ect/imgui/imgui.h"
#include <windows.h>
#include "config.h"
#include <iostream>
#include "imgui.h"

bool WorldToScreen(Vec3 pos, float matrix[16], Vec2& screen, int screenWidth, int screenHeight) {

    float clipX = matrix[0] * pos.x + matrix[4] * pos.y + matrix[8] * pos.z + matrix[12];
    float clipY = matrix[1] * pos.x + matrix[5] * pos.y + matrix[9] * pos.z + matrix[13];
    float clipZ = matrix[2] * pos.x + matrix[6] * pos.y + matrix[10] * pos.z + matrix[14];
    float clipW = matrix[3] * pos.x + matrix[7] * pos.y + matrix[11] * pos.z + matrix[15];


    if (clipW < 0.1f) {
        return false;
    }


    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;


    screen.x = (screenWidth / 2.0f) * (ndcX + 1.0f);
    screen.y = (screenHeight / 2.0f) * (1.0f - ndcY);

    return true;
}


void RenderESP(bool teamesp, HANDLE hProcess, uintptr_t moduleBase, int screenWidth, int screenHeight, bool snaplines, bool snaplines_all, bool esp_names) {

    ImVec4 esp_color = Config::getenemyespcolor();
    

    uintptr_t localPlayer = ReadMemory<uintptr_t>(hProcess, moduleBase + Offsets::LOCALPLAYER);
    uintptr_t entityList = ReadMemory<uintptr_t>(hProcess, moduleBase + Offsets::ENTITYLIST);


    float viewMatrix[16];
    ReadProcessMemory(hProcess, (LPCVOID)Offsets::VIEWMATRIX, viewMatrix, sizeof(viewMatrix), nullptr);

    if (!localPlayer) return;

    int localTeam = ReadMemory<int>(hProcess, localPlayer + Offsets::TEAM);
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();


    for (int i = 0; i < 64; i++) {
        uintptr_t entity = ReadMemory<uintptr_t>(hProcess, entityList + i * 0x4);
        if (!entity || entity == localPlayer) continue;


        int health = ReadMemory<int>(hProcess, entity + Offsets::HEALTH);
        if (health <= 0 || health > 100) continue;

        int armor = ReadMemory<int>(hProcess, entity + Offsets::ARMOR);
        if (!entity || entity == localPlayer) continue;

        int team = ReadMemory<int>(hProcess, entity + Offsets::TEAM);
        if (team == localTeam && !teamesp) continue;

        if(team == 4) continue; //dont render spectators


        Vec3 head;
        head.x = ReadMemory<float>(hProcess, entity + Offsets::HEAD_X);
        head.y = ReadMemory<float>(hProcess, entity + Offsets::HEAD_Y);
        head.z = ReadMemory<float>(hProcess, entity + Offsets::HEAD_Z);


        Vec3 feet;
        feet.x = ReadMemory<float>(hProcess, entity + Offsets::FEET_X);
        feet.y = ReadMemory<float>(hProcess, entity + Offsets::FEET_Y);
        feet.z = ReadMemory<float>(hProcess, entity + Offsets::FEET_Z);









        Vec2 headScreen, feetScreen;
        if (!WorldToScreen(head, viewMatrix, headScreen, screenWidth, screenHeight)) continue;
        if (!WorldToScreen(feet, viewMatrix, feetScreen, screenWidth, screenHeight)) continue;


        float boxHeight = feetScreen.y - headScreen.y;
        float boxWidth = boxHeight / 2.0f;

        drawList->AddRect(
            ImVec2(headScreen.x - boxWidth / 2, headScreen.y),
            ImVec2(headScreen.x + boxWidth / 2, feetScreen.y),
            teamesp && team == localTeam ? IM_COL32(0, 255, 0, 255) : IM_COL32(
                static_cast<int>(esp_color.x * 255),
                static_cast<int>(esp_color.y * 255),
                static_cast<int>(esp_color.z * 255),
                static_cast<int>(esp_color.w * 255)
            )
        );

        // i only read names if we have esp names on :)
        if(esp_names){
            char name[17] = {0};
            ReadProcessMemory(hProcess, (LPCVOID)(entity + 0x205), &name, 16, nullptr);
            if (name[0] != '\0') {
                drawList->AddText(ImVec2(headScreen.x, headScreen.y - 15), IM_COL32(255,255,255,255), name);
            }
        }

        char armorText[16];
        snprintf(armorText, sizeof(armorText), "armor: %d", armor);

        drawList->AddText(
            ImVec2(feetScreen.x, feetScreen.y), IM_COL32(7,0,207,255), armorText);


        //char team_debug_Text[15];
        //snprintf(team_debug_Text, sizeof(team_debug_Text), "DEBUG, TEAM: %d", team);       //DEBUG CODE FOR RENDERIRING TEAM OF PLAYER

//        drawList->AddText(
//            ImVec2(feetScreen.x, feetScreen.y - 25), IM_COL32(7,0,207,255), team_debug_Text);
            

        drawList->AddRectFilled(
            ImVec2(headScreen.x - boxWidth / 2 - 6, headScreen.y),
            ImVec2(headScreen.x - boxWidth / 2 - 3, feetScreen.y),
            IM_COL32(0, 0, 0, 255)
        );

        if (team != localTeam && snaplines) {
            drawList->AddLine(
                ImVec2(screenWidth / 2.0f, 0),
                ImVec2(headScreen.x, headScreen.y),
                IM_COL32(255, 255, 0, 255), // enemy color
                1.0f
            );
        } else if (team == localTeam && snaplines_all) {
            drawList->AddLine(
                ImVec2(screenWidth / 2.0f, 0),
                ImVec2(headScreen.x, headScreen.y),
                IM_COL32(0, 255, 0, 255), // teammate color
                1.0f
            );
        }


        float healthPercent = health / 100.0f;
        ImU32 healthColor = IM_COL32(
            (int)(255 * (1 - healthPercent)),  
            (int)(255 * healthPercent),     
            0,
            255
        );






        drawList->AddRectFilled(
            ImVec2(headScreen.x - boxWidth / 2 - 8, headScreen.y),
            ImVec2(headScreen.x - boxWidth / 2 - 6, feetScreen.y),
            IM_COL32(0, 0, 0, 255)
        );
        drawList->AddRectFilled(
            ImVec2(headScreen.x - boxWidth / 2 - 8, headScreen.y + boxHeight * (1 - healthPercent)),
            ImVec2(headScreen.x - boxWidth / 2 - 6, feetScreen.y),
            healthColor
        );


    }
}
