#pragma once

#include "imgui.h"

#include <cstdint>

class Display
{
public:
    virtual void Draw() = 0;
    Display(ImVec2 size);
    virtual ~Display();

protected:
    ImVec2 mDisplaySize;
    ImGuiContext* mContext;
};

class TV : public Display
{
public:
    void Draw() override;
    TV(ImVec2 size) : Display(size) {}
};

class DRC : public Display
{
public:
    void Draw() override;
    DRC(ImVec2 size) : Display(size) {}
};