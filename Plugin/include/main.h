#pragma once

#include "overlay.h"

extern struct GX2ContextState* gContextState;
extern bool gOverlayInitDone;

TV* GetTV();
DRC* GetDRC();

void SetTV(TV* t);
void SetDRC(DRC* d);