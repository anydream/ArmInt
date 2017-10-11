#pragma once

#include <stdint.h>

#if TARGET_64_BIT
typedef uint64_t Ptr_t;
#else
typedef uint32_t Ptr_t;
#endif
