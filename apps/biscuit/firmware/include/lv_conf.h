#ifndef LV_CONF_H
#define LV_CONF_H
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC malloc
#define LV_MEM_CUSTOM_FREE free
#define LV_MEM_CUSTOM_REALLOC realloc
#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 20
#define LV_DPI_DEF 160
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(biscuit_font_16) LV_FONT_DECLARE(biscuit_font_20) LV_FONT_DECLARE(biscuit_font_24) LV_FONT_DECLARE(biscuit_font_28)
#define LV_FONT_DEFAULT &biscuit_font_20
#define LV_USE_LOG 0
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_BUILD_EXAMPLES 0
#define LV_USE_DEMO_WIDGETS 0
#endif
