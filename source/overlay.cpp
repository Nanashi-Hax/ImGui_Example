#include "overlay.h"
#include "main.h"

#include "imgui.h"
#include "imgui_backend/imgui_impl_gx2.h"
#include "imgui_backend/imgui_impl_wiiu.h"

#include <gx2/state.h>

Display::Display(ImVec2 displaySize)
{
    IMGUI_CHECKVERSION();

    mContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(mContext);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.DisplaySize = displaySize;

    ImGui::StyleColorsDark();

    ImGui_ImplWiiU_Init();
    ImGui_ImplGX2_Init();
}

Display::~Display()
{
    ImGui::DestroyContext(mContext);
}

void TV::Draw()
{
    ImGui::SetCurrentContext(mContext);

    ImGui_ImplGX2_NewFrame();
    ImGui::NewFrame();

    ImGui::Render();
    ImGui_ImplGX2_RenderDrawData(ImGui::GetDrawData());

    GX2Flush();
}

void DRC::Draw()
{
    ImGui::SetCurrentContext(mContext);

    ImGui_ImplGX2_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplGX2_RenderDrawData(ImGui::GetDrawData());

    GX2Flush();
}