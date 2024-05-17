#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <zephyr/kernel.h>

typedef bool BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef long LONG;

#if (!defined(TRUE))
#define TRUE true
#endif

#if (!defined(FALSE))
#define FALSE false
#endif

#endif
