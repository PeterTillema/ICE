#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define RET_A         (1<<7)
#define UN            (1<<6)
#define RET_HLs       (1<<5)
#define RET_NONE      (0)
#define RET_HL        (0)
#define ARG_NORM      (0)
#define ARG_1         (1<<7)
#define ARG_2         (1<<6)
#define ARG_3         (1<<5)
#define ARG_4         (1<<4)
#define ARG_5         (1<<3)
#define ARG_12        (ARG_1 | ARG_2)
#define ARG_123       (ARG_1 | ARG_2 | ARG_3)
#define ARG_13        (ARG_1 | ARG_3)
#define ARG_23        (ARG_2 | ARG_3)
#define ARG_14        (ARG_1 | ARG_4)
#define ARG_24        (ARG_2 | ARG_4)
#define ARG_45        (ARG_4 | ARG_5)
#define ARG_34        (ARG_3 | ARG_4)
#define ARG_345       (ARG_3 | ARG_45)

#define TI_ISARCHIVED_INDEX 12
#define TI_TELL_INDEX       14
#define TI_GETSIZE_INDEX    16
#define TI_GETDATAPTR_INDEX 18
#define TI_GETVATPTR_INDEX  30
#define TI_GETNAME_INDEX    31

uint24_t executeFunction(NODE*);
uint8_t parseFunction(NODE*);
uint8_t parseFunction2Args(NODE*, NODE*, uint8_t, bool);
void LoadVariableInReg(uint8_t, uint8_t);
void LoadValueInReg(uint8_t, uint24_t, uint8_t);
uint8_t InsertDataElements(NODE*);
void loadGetKeyFastData1(void);
void loadGetKeyFastData2(void);
void InsertMallocRoutine(void);

uint8_t functionLParen(NODE*);
uint8_t functionMinMax(NODE*);
uint8_t functionNot(NODE*);
uint8_t functionSinCos(NODE*);
uint8_t functionRand(NODE*);
uint8_t functionSqrt(NODE*);
uint8_t functionMean(NODE*);
uint8_t functionAns(NODE*);
uint8_t functionBrace(NODE*);
uint8_t functionRandInt(NODE*);
uint8_t functionSub(NODE*);
uint8_t functionC(NODE*);
uint8_t functionToString(NODE*);
uint8_t functionLength(NODE*);
uint8_t functionLEFTRIGHT(NODE*);
uint8_t functionStartTmr(NODE*);
uint8_t functionRemainder(NODE*);
uint8_t functionCheckTmr(NODE*);
uint8_t functionGetKey(NODE*);
uint8_t functionDbd(NODE*);
uint8_t functionAlloc(NODE*);
uint8_t functionDefineSprite(NODE*);
uint8_t functionData(NODE*);
uint8_t functionCopy(NODE*);
uint8_t functionCopyData(NODE*);
uint8_t functionDefineTilemap(NODE*);
uint8_t functionLoadData(NODE*);
uint8_t functionSetBrightness(NODE*);

typedef struct {
    uint8_t function;
    uint8_t function2;
    uint8_t amountOfArgs;
    uint8_t numbersArgs;
    uint8_t pushBackwards;
    uint8_t parseStandardArgs;
    uint8_t (*exprFunction)(NODE*);
} function_t;

typedef struct {
    uint8_t retRegAndArgs;
    uint8_t smallArgs;
} c_function_t;

#endif
