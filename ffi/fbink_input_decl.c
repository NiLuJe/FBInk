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

// Typedefs
cdecl_type(INPUT_DEVICE_TYPE_E)
cdecl_c99_type(INPUT_DEVICE_TYPE_T, uint32_t)
cdecl_type(INPUT_SETTINGS_TYPE_E)
cdecl_c99_type(INPUT_SETTINGS_TYPE_T, uint32_t)


// Config structs
cdecl_type(FBInkInputDevice)

// API
cdecl_func(fbink_input_scan)
cdecl_func(fbink_input_check)
