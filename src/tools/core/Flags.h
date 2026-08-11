#pragma once

#define HAS_FLAGS(current_flags, flags)    ((current_flags & flags) == flags)
#define SET_FLAGS(current_flags, flags, value ) current_flags = (current_flags & ~flags) | (((bool)(value)) * flags)
