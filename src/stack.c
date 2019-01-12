#include "defines.h"
#include "stack.h"

#include "main.h"

static unsigned int *p1;
static unsigned int *p2;

void push(unsigned int i) {
    *p1++ = i;
}

unsigned int getNextIndex(void) {
    return *(p2++);
}

unsigned int getIndexOffset(int offset) {
    return *(p2 + offset);
}

void removeIndexFromStack(int index) {
    memmove(ice.stackStart + index, ice.stackStart + index + 1, (STACK_SIZE - index - 1) * sizeof(unsigned int));
    p2--;
}

int getCurrentIndex(void) {
    return p2 - ice.stackStart;
}

unsigned int *getStackVar(uint8_t which) {
    if (which) {
        return p2;
    }
    return p1;
}

void setStackValues(unsigned int *val1, unsigned int *val2) {
    p1 = val1;
    p2 = val2;
}