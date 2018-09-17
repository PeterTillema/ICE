#ifndef OPERATOR_H
#define OPERATOR_H

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

bool comparePtrToTempStrings(uint24_t);
uint8_t getIndexOfOperator(uint8_t);
uint24_t executeOperator(NODE*);
uint8_t parseOperator(NODE*);

void EQInsert(void);
void GEInsert(void);

void MultWithNumber(uint24_t, uint8_t*, bool);

extern const char operators[];
extern const uint8_t operatorPrecedence[];
extern const uint8_t operatorPrecedence2[];

#endif
