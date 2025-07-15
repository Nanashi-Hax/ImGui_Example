#include <wups.h>
#include "main.h"
#include "overlay.h"

#include <gx2/context.h>
#include <gx2/enum.h>
#include <gx2/surface.h>
#include <gx2/registers.h>
#include <coreinit/cache.h>
#include <gx2/mem.h>

#include <cstring> // for memcpy
#include <gx2/state.h>

#include "imgui_backend/imgui_impl_wiiu.h"

namespace
{
    GX2SurfaceFormat tvSurfaceFormat = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    GX2SurfaceFormat drcSurfaceFormat = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
    GX2ContextState* gOriginalContextState = nullptr;
    GX2ColorBuffer lastTVColorBuffer;
    GX2ColorBuffer lastDRCColorBuffer;
}

bool drawScreenshotSavedTexture(const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target);

DECL_FUNCTION(void, GX2GetCurrentScanBuffer, GX2ScanTarget scanTarget, GX2ColorBuffer *cb)
{
    real_GX2GetCurrentScanBuffer(scanTarget, cb);
    if (scanTarget == GX2_SCAN_TARGET_TV) {
        memcpy(&lastTVColorBuffer, cb, sizeof(GX2ColorBuffer));
    } else {
        memcpy(&lastDRCColorBuffer, cb, sizeof(GX2ColorBuffer));
    }
}

DECL_FUNCTION(void, GX2SetContextState, GX2ContextState *curContext)
{
    real_GX2SetContextState(curContext);

    gOriginalContextState = curContext;
}

DECL_FUNCTION(void, GX2SetupContextStateEx, GX2ContextState *state, BOOL unk1)
{
    real_GX2SetupContextStateEx(state, unk1);
    gOriginalContextState = state;
}

DECL_FUNCTION(void, GX2SetTVBuffer, void *buffer, uint32_t buffer_size, int32_t tv_render_mode, GX2SurfaceFormat surface_format, GX2BufferingMode buffering_mode)
{
    tvSurfaceFormat = surface_format;

    return real_GX2SetTVBuffer(buffer, buffer_size, tv_render_mode, surface_format, buffering_mode);
}

DECL_FUNCTION(void, GX2SetDRCBuffer, void *buffer, uint32_t buffer_size, uint32_t drc_mode, GX2SurfaceFormat surface_format, GX2BufferingMode buffering_mode)
{
    drcSurfaceFormat = surface_format;

    return real_GX2SetDRCBuffer(buffer, buffer_size, drc_mode, surface_format, buffering_mode);
}

void drawIntoColorBuffer(const GX2ColorBuffer* colorBuffer, GX2ScanTarget scan_target) {
    real_GX2SetContextState(gContextState);


    GX2SetDefaultState();

    GX2SetColorBuffer(colorBuffer, GX2_RENDER_TARGET_0);
    GX2SetViewport(0.0f, 0.0f, colorBuffer->surface.width, colorBuffer->surface.height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, colorBuffer->surface.width, colorBuffer->surface.height);

    GX2SetDepthOnlyControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_FUNC_NEVER);
    GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_FUNC_GREATER, 0.0f);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);

    Overlay_Draw(colorBuffer->surface.width, colorBuffer->surface.height);

    GX2Flush();

    real_GX2SetContextState(gOriginalContextState);
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target)
{
    gDrawReady = true;
    if (drawScreenshotSavedTexture(colorBuffer, scan_target))
    {
        // if it returns true we don't need to call GX2CopyColorBufferToScanBuffer
        return;
    }

    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

static inline void GX2InitColorBuffer(GX2ColorBuffer* colorBuffer, GX2SurfaceDim dim, uint32_t width, uint32_t height,
                                      uint32_t depth, GX2SurfaceFormat format, GX2AAMode aa, GX2TileMode tilemode,
                                      uint32_t swizzle,
                                      void* aaBuffer, uint32_t aaSize)
{
    colorBuffer->surface.dim = dim;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = depth;
    colorBuffer->surface.mipLevels = 1;
    colorBuffer->surface.format = format;
    colorBuffer->surface.aa = aa;
    colorBuffer->surface.use = GX2_SURFACE_USE_TEXTURE_COLOR_BUFFER_TV;
    colorBuffer->surface.imageSize = 0;
    colorBuffer->surface.image = nullptr;
    colorBuffer->surface.mipmapSize = 0;
    colorBuffer->surface.mipmaps = nullptr;
    colorBuffer->surface.tileMode = tilemode;
    colorBuffer->surface.swizzle = swizzle;
    colorBuffer->surface.alignment = 0;
    colorBuffer->surface.pitch = 0;
    uint32_t i;
    for (i = 0; i < 13; i++)
        colorBuffer->surface.mipLevelOffset[i] = 0;
    colorBuffer->viewMip = 0;
    colorBuffer->viewFirstSlice = 0;
    colorBuffer->viewNumSlices = depth;
    colorBuffer->aaBuffer = aaBuffer;
    colorBuffer->aaSize = aaSize;
    for (i = 0; i < 5; i++)
        colorBuffer->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&colorBuffer->surface);
    GX2InitColorBufferRegs(colorBuffer);
}

bool drawScreenshotSavedTexture(const GX2ColorBuffer* colorBuffer, GX2ScanTarget scan_target)
{
    GX2ColorBuffer cb;
    GX2InitColorBuffer(&cb,
                       colorBuffer->surface.dim,
                       colorBuffer->surface.width,
                       colorBuffer->surface.height,
                       colorBuffer->surface.depth,
                       colorBuffer->surface.format,
                       colorBuffer->surface.aa,
                       colorBuffer->surface.tileMode,
                       colorBuffer->surface.swizzle,
                       colorBuffer->aaBuffer,
                       colorBuffer->aaSize);

    cb.surface.image = colorBuffer->surface.image;

    drawIntoColorBuffer(&cb, scan_target);

    real_GX2CopyColorBufferToScanBuffer(&cb, scan_target);

    return true;
}

void drawScreenshotSavedTexture2(GX2ColorBuffer* colorBuffer, GX2ScanTarget scan_target)
{
    drawIntoColorBuffer(colorBuffer, scan_target);
}

DECL_FUNCTION(void, GX2Init, uint32_t attributes)
{
    real_GX2Init(attributes);
    if (!gOverlayInitDone)
    {
        // has been allocated in INITIALIZE_PLUGIN
        if (!gContextState)
        {
            OSFatal("Failed to alloc gContextState");
        }
        real_GX2SetupContextStateEx(gContextState, GX2_TRUE);
        GX2Invalidate(GX2_INVALIDATE_MODE_CPU, gContextState, sizeof(GX2ContextState));
        DCInvalidateRange(gContextState, sizeof(GX2ContextState)); // Important!
        gOverlayInitDone = true;
    }
}

DECL_FUNCTION(void, GX2MarkScanBufferCopied, GX2ScanTarget scan_target)
{
    gDrawReady = true;
    if (scan_target == GX2_SCAN_TARGET_TV) {
        drawScreenshotSavedTexture2(&lastTVColorBuffer, scan_target);
    } else {
        drawScreenshotSavedTexture2(&lastDRCColorBuffer, scan_target);
    }

    real_GX2MarkScanBufferCopied(scan_target);
}

DECL_FUNCTION(void, GX2SwapScanBuffers, void) {
    real_GX2SwapScanBuffers();
}

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus* buffers, uint32_t count, VPADReadError* error) {
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffers, count, &real_error);

    if (result > 0 && real_error == VPAD_READ_SUCCESS) {
        if (ImGui_ImplWiiU_ProcessVPADInput(&buffers[0])) {
            // If we're here ImGui has captured the touch and we clear it for all other applications
            for (uint32_t i = 0; i < count; i++) {
                buffers[i].tpNormal.touched = 0;
                buffers[i].tpFiltered1.touched = 0;
                buffers[i].tpFiltered2.touched = 0;
            }
        }
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

WUPS_MUST_REPLACE(GX2GetCurrentScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2GetCurrentScanBuffer);
WUPS_MUST_REPLACE(GX2SetContextState, WUPS_LOADER_LIBRARY_GX2, GX2SetContextState);
WUPS_MUST_REPLACE(GX2SetupContextStateEx, WUPS_LOADER_LIBRARY_GX2, GX2SetupContextStateEx);
WUPS_MUST_REPLACE(GX2SetTVBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetTVBuffer);
WUPS_MUST_REPLACE(GX2SetDRCBuffer, WUPS_LOADER_LIBRARY_GX2, GX2SetDRCBuffer);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(GX2Init, WUPS_LOADER_LIBRARY_GX2, GX2Init);
WUPS_MUST_REPLACE(GX2MarkScanBufferCopied, WUPS_LOADER_LIBRARY_GX2, GX2MarkScanBufferCopied);
WUPS_MUST_REPLACE(GX2SwapScanBuffers, WUPS_LOADER_LIBRARY_GX2, GX2SwapScanBuffers);

WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
