// --- RGB modes (single source of truth) ---
#ifndef RGB_MODES_DEFINED
#define RGB_MODES_DEFINED
enum RGBMode { RGB_MODE_AUTO = 0, RGB_MODE_MANUAL = 1, RGB_MODE_OFF = 2 };
#endif
#pragma once
#include "pins_local.h"
#include "espnow.h"
#include "ESPping.h"