#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include <stdint.h>
#ifdef __EMSCRIPTEN__
#include "tice.h"
#else
#include <tice.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _getc()      getNextToken()

#if !defined(COMPUTER_ICE) && !defined(__EMSCRIPTEN__)

#define CALCULATOR

#include <fileioc.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
#define w24(x, y) (*(uint24_t*)(x) = y)
#define r24(x) (*(uint24_t*)(x))
#define _rewind(x)   ti_Rewind(x)
#define _close(x)    ti_Close(x)
#define _open(x)     ti_OpenVar(x, "r", TI_PRGM_TYPE)
#define _new(x)      ti_OpenVar(x, "w", TI_PPRGM_TYPE)
#define _tell(x)     ti_Tell(x)
#define _seek(x,y,z) ti_Seek(x,y,z)

#else

#define TI_PRGM_TYPE 5
#define M_PI         3.14159265358979323846

void w24(void *x, uint32_t val);
void w16(void *x, uint32_t val);
uint32_t r24(void *x);
void export_program(const char *name, uint8_t *data, size_t size);

typedef FILE* ti_var_t;
typedef int32_t int24_t;
typedef uint32_t uint24_t;

#define _rewind(x)   fseek(x, 0x4A, SEEK_SET)
#define _open(x)     fopen(x, "rb")
#define _close(x)    fclose(x)
#define _tell(x)     ftell(x)
#define _seek(x,y,z) fseek(z,x,y);

#endif

#endif
