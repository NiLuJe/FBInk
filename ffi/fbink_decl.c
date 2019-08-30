// For use with https://github.com/koreader/ffi-cdecl to re-generate up-to-date FFI declarations ;)
#include <fbink.h>

#include "ffi-cdecl.h"

// Keep this in the same order as fbink.h

// Constants
cdecl_const(FBFD_AUTO)

// Typedefs
cdecl_type(FONT_INDEX_T)
cdecl_type(FONT_STYLE_T)

cdecl_type(ALIGN_INDEX_T)

cdecl_type(FG_COLOR_INDEX_T)
cdecl_type(BG_COLOR_INDEX_T)

cdecl_type(WFM_MODE_INDEX_T)
cdecl_type(HW_DITHER_INDEX_T)

cdecl_type(NTX_ROTA_INDEX_T)

// Config structs
cdecl_type(FBInkState)

cdecl_type(FBInkConfig)

cdecl_type(FBInkOTConfig)
cdecl_type(FBInkOTFit)

cdecl_type(FBInkRect)

cdecl_type(FBInkDump)

// API
cdecl_func(fbink_version)

cdecl_func(fbink_open)
cdecl_func(fbink_close)
cdecl_func(fbink_init)

cdecl_func(fbink_state_dump)
cdecl_func(fbink_get_state)

cdecl_func(fbink_print)

cdecl_func(fbink_add_ot_font)
cdecl_func(fbink_free_ot_fonts)
cdecl_func(fbink_print_ot)

cdecl_func(fbink_printf)

cdecl_func(fbink_refresh)

//cdecl_func(fbink_is_fb_quirky)
cdecl_func(fbink_reinit)

cdecl_func(fbink_print_progress_bar)
cdecl_func(fbink_print_activity_bar)

cdecl_func(fbink_print_image)
cdecl_func(fbink_print_raw_data)

cdecl_func(fbink_cls)

cdecl_func(fbink_dump)
cdecl_func(fbink_region_dump)
cdecl_func(fbink_restore)
cdecl_func(fbink_free_dump_data)

cdecl_func(fbink_get_last_rect)

cdecl_func(fbink_button_scan)
cdecl_func(fbink_wait_for_usbms_processing)
