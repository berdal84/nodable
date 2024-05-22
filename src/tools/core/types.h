#pragma once

// integers
typedef long long i64_t;
typedef int   i32_t;
typedef short i16_t;
typedef char  i8_t;

static_assert( sizeof(i64_t) == 8, "Type has wrong size"  );
static_assert( sizeof(i32_t) == 4, "Type has wrong size"  );
static_assert( sizeof(i16_t) == 2, "Type has wrong size"  );
static_assert( sizeof(i8_t)  == 1, "Type has wrong size"  );

// unsigned integers
typedef unsigned long long u64_t;
typedef unsigned int       u32_t;
typedef unsigned short     u16_t;
typedef unsigned char      u8_t;

static_assert( sizeof(u64_t) == 8, "Type has wrong size"  );
static_assert( sizeof(u32_t) == 4, "Type has wrong size"  );
static_assert( sizeof(u16_t) == 2, "Type has wrong size"  );
static_assert( sizeof(u8_t)  == 1, "Type has wrong size"  );




