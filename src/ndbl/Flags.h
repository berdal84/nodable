#pragma once

#define HAS_FLAGS(current_flags, flags)     ((current_flags & (flags)) == (flags))
#define UNSET_FLAGS(current_flags, flags)   current_flags &= ~flags
#define SET_FLAGS(current_flags, flags)     current_flags |= flags
#define TOGGLE_FLAGS(current_flags, flags)  current_flags ^= flags
#define SET_FLAGS_VALUE(current_flags, flags, value ) current_flags = (current_flags & ~flags) | (((bool)(value)) * flags)
