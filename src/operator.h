#ifndef OPERATOR_H
#define OPERATOR_H

#include "ast.h"

enum {
    REGISTER_HL,
    REGISTER_DE,
    REGISTER_BC,
    REGISTER_HL_DE,
    REGISTER_A
};

enum {
    TempString1,
    TempString2
};

#define NO_PUSH   false
#define NEED_PUSH true

bool comparePtrToTempStrings(unsigned int);

uint8_t getIndexOfOperator(uint8_t);

unsigned int executeOperator(unsigned int, unsigned int, uint8_t);

void LD_HL_STRING(unsigned int, uint8_t);

uint8_t parseOperator(element_t *, element_t *, element_t *, element_t *, bool);

void StoToChainAns(void);

void EQInsert(void);

void GEInsert(void);

void AddStringString(void);

void StoStringString(void);

void StoStringVariable(void);

void StoStringChainAns(void);

void MultWithNumber(unsigned int, uint8_t *, bool);

extern const char operators[];
extern const uint8_t operatorPrecedence[];
extern const uint8_t operatorPrecedence2[];

#endif
