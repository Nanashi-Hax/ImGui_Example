#include "main.h"
#include "overlay.h"

#include <wups.h>
#include <memory/mappedmemory.h>

#include <gx2/context.h>
#include <gx2/enum.h>
#include <whb/log_udp.h>
#include <whb/log.h>

WUPS_PLUGIN_NAME("ImGui Example");
WUPS_PLUGIN_DESCRIPTION("Overlay Plugin");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi-Hax");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_STORAGE("ImGui"); // Required for ON_APPLICATION_REQUESTS_EXIT to be called

GX2ContextState* gContextState = nullptr;
bool gOverlayInitDone = false;

namespace
{
    TV* tv;
    DRC* drc;
}

INITIALIZE_PLUGIN()
{
    gOverlayInitDone = false;

    gContextState = (GX2ContextState *)MEMAllocFromMappedMemoryForGX2Ex
    (
        sizeof(GX2ContextState),
        GX2_CONTEXT_STATE_ALIGNMENT
    );

    if (gContextState == nullptr)
    {
        OSFatal("Failed to allocate gContextState");
    }
}

ON_APPLICATION_START()
{
    WHBLogUdpInit();
    WHBLogPrintf("Initializing ImGui...");

    SetTV(new TV(ImVec2(1280.0f, 720.0f)));
    SetDRC(new DRC(ImVec2(854.0f, 480.0f)));
}

ON_APPLICATION_REQUESTS_EXIT()
{
    delete GetTV();
    delete GetDRC();

    WHBLogPrintf("Shutting down ImGui...");
    WHBLogUdpDeinit();
}

TV* GetTV()
{
    return tv;
}

DRC* GetDRC()
{
    return drc;
}

void SetTV(TV* t)
{
    tv = t;
}

void SetDRC(DRC* d)
{
    drc = d;
}