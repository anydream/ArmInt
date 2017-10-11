#pragma once

#include <stdint.h>

#if TARGET_64_BIT
typedef uint64_t Ptr_t;
#else
typedef uint32_t Ptr_t;
#endif

#ifndef HOST_BIG_ENDIAN
#define HOST_BIG_ENDIAN 0
#endif
