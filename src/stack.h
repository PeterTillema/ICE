#ifndef STACK_H
#define STACK_H

void push(unsigned int);

unsigned int pop(void);

unsigned int getStackSize(void);

unsigned int getNextIndex(void);

int getCurrentIndex(void);

unsigned int getIndexOffset(int);

void removeIndexFromStack(int);

void getNextFreeStack(void);

void removeStack(void);

unsigned int *getStackVar(uint8_t);

void setStackValues(unsigned int *, unsigned int *);

#endif
