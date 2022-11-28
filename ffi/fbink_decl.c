// For use with https://github.com/koreader/ffi-cdecl to re-generate up-to-date FFI declarations ;)
#include <fbink.h>

#include "ffi-cdecl.h"

// Keep this in the same order as fbink.h
// Mostly to make it easier to update, but also because Lua can be finicky with the order in which stuff is declared.
// NOTE: Python's cFFI cdef() function doesn't support attributes (c.f., https://cffi.readthedocs.io/en/latest/cdef.html#ffi-cdef-limitations).
//       That means a bit of manual cleanup will be needed there ;).
//       But this isn't an actual issue in practice for us, though:
//       there's a const function attribute, and the packed enums which we only access through a properly typed typedef.

// Constants
cdecl_const(FBFD_AUTO)
cdecl_const(LAST_MARKER)

// Typedefs
cdecl_type(FONT_INDEX_E)
cdecl_c99_type(FONT_INDEX_T, uint8_t)

cdecl_type(FONT_STYLE_E)
cdecl_type(FONT_STYLE_T)

cdecl_type(ALIGN_INDEX_E)
cdecl_c99_type(ALIGN_INDEX_T, uint8_t)
cdecl_type(PADDING_INDEX_E)
cdecl_c99_type(PADDING_INDEX_T, uint8_t)

cdecl_type(FG_COLOR_INDEX_E)
cdecl_c99_type(FG_COLOR_INDEX_T, uint8_t)
cdecl_type(BG_COLOR_INDEX_E)
cdecl_c99_type(BG_COLOR_INDEX_T, uint8_t)

cdecl_type(CERVANTES_DEVICE_ID_E)
cdecl_type(KOBO_DEVICE_ID_E)
cdecl_type(MAINLINE_DEVICE_ID_E)
cdecl_type(REMARKABLE_DEVICE_ID_E)
cdecl_type(POCKETBOOK_DEVICE_ID_E)
cdecl_c99_type(DEVICE_ID_T, uint16_t)
cdecl_const(DEVICE_UNKNOWN)

cdecl_type(WFM_MODE_INDEX_E)
cdecl_c99_type(WFM_MODE_INDEX_T, uint8_t)
cdecl_type(HW_DITHER_INDEX_E)
cdecl_c99_type(HW_DITHER_INDEX_T, uint8_t)

cdecl_type(NTX_ROTA_INDEX_E)
cdecl_c99_type(NTX_ROTA_INDEX_T, uint8_t)

cdecl_type(SUNXI_FORCE_ROTA_INDEX_E)
cdecl_c99_type(SUNXI_FORCE_ROTA_INDEX_T, int8_t)

cdecl_type(MTK_SWIPE_DIRECTION_INDEX_E)
cdecl_c99_type(MTK_SWIPE_DIRECTION_INDEX_T, uint8_t)
cdecl_type(MTK_HALFTONE_MODE_INDEX_E)
cdecl_c99_type(MTK_HALFTONE_MODE_INDEX_T, int32_t)

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
cdecl_func(fbink_add_ot_font_v2)
cdecl_func(fbink_free_ot_fonts)
cdecl_func(fbink_free_ot_fonts_v2)
cdecl_func(fbink_print_ot)

cdecl_func(fbink_printf)

cdecl_func(fbink_refresh)
cdecl_func(fbink_wait_for_submission)
cdecl_func(fbink_wait_for_complete)
cdecl_func(fbink_get_last_marker)

//cdecl_func(fbink_is_fb_quirky)
cdecl_const(OK_BPP_CHANGE)
cdecl_const(OK_ROTA_CHANGE)
cdecl_const(OK_LAYOUT_CHANGE)
cdecl_const(OK_GRAYSCALE_CHANGE)
cdecl_func(fbink_reinit)

cdecl_func(fbink_update_verbosity)

cdecl_func(fbink_update_pen_colors)
cdecl_const(OK_ALREADY_SAME)
cdecl_func(fbink_set_fg_pen_gray)
cdecl_func(fbink_set_bg_pen_gray)
cdecl_func(fbink_set_fg_pen_rgba)
cdecl_func(fbink_set_bg_pen_rgba)

cdecl_func(fbink_print_progress_bar)
cdecl_func(fbink_print_activity_bar)

cdecl_func(fbink_print_image)
cdecl_func(fbink_print_raw_data)

cdecl_func(fbink_cls)

cdecl_func(fbink_grid_clear)
cdecl_func(fbink_grid_refresh)

cdecl_func(fbink_dump)
cdecl_func(fbink_region_dump)
cdecl_func(fbink_rect_dump)
cdecl_func(fbink_restore)
cdecl_func(fbink_free_dump_data)

cdecl_func(fbink_get_last_rect)

cdecl_func(fbink_button_scan)
cdecl_func(fbink_wait_for_usbms_processing)

cdecl_func(fbink_rota_native_to_canonical)
cdecl_func(fbink_rota_canonical_to_native)

cdecl_func(fbink_invert_screen)

cdecl_func(fbink_get_fb_pointer)
cdecl_func(fbink_get_fb_info)
cdecl_const(KEEP_CURRENT_ROTATE)
cdecl_const(KEEP_CURRENT_BITDEPTH)
cdecl_const(KEEP_CURRENT_GRAYSCALE)
cdecl_const(TOGGLE_GRAYSCALE)
cdecl_func(fbink_set_fb_info)

cdecl_func(fbink_sunxi_toggle_ntx_pen_mode)
cdecl_func(fbink_sunxi_ntx_enforce_rota)

cdecl_func(fbink_mtk_set_swipe_data)
cdecl_func(fbink_wait_for_any_complete)
cdecl_func(fbink_mtk_set_halftone)
cdecl_func(fbink_mtk_toggle_auto_reagl)
cdecl_func(fbink_mtk_toggle_pen_mode)
