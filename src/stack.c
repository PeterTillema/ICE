#include "defines.h"
#include "stack.h"

#include "ast.h"
#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"
#include "functions.h"
#include "routines.h"

static uint24_t *p1;
static uint24_t *p2;

void push(uint24_t i) {
    *p1++ = i;
}

uint24_t getNextIndex(void) {
    return *(p2++);
}

uint24_t getIndexOffset(int24_t offset) {
    return *(p2 + offset);
}

void removeIndexFromStack(int24_t index) {
    memmove(ice.stackStart + index, ice.stackStart + index + 1, (STACK_SIZE - index - 1) * sizeof(uint24_t));
    p2--;
}

int24_t getCurrentIndex(void) {
    return p2 - ice.stackStart;
}

uint24_t *getStackVar(uint8_t which) {
    if (which) {
        return p2;
    }
    return p1;
}

void setStackValues(uint24_t* val1, uint24_t* val2) {
    p1 = val1;
    p2 = val2;
}