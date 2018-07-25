#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define RET_A         (1<<7)
#define UN            (1<<6)
#define RET_HLs       (1<<5)
#define RET_NONE      (0)
#define RET_HL        (0)
#define ARG_NORM      (0)
#define SMALL_1       (1<<7)
#define SMALL_2       (1<<6)
#define SMALL_3       (1<<5)
#define SMALL_4       (1<<4)
#define SMALL_5       (1<<3)
#define SMALL_12      (SMALL_1 | SMALL_2)
#define SMALL_123     (SMALL_1 | SMALL_2 | SMALL_3)
#define SMALL_13      (SMALL_1 | SMALL_3)
#define SMALL_23      (SMALL_2 | SMALL_3)
#define SMALL_14      (SMALL_1 | SMALL_4)
#define SMALL_24      (SMALL_2 | SMALL_4)
#define SMALL_45      (SMALL_4 | SMALL_5)
#define SMALL_34      (SMALL_3 | SMALL_4)
#define SMALL_345     (SMALL_3 | SMALL_45)

#define TI_ISARCHIVED_INDEX 12
#define TI_TELL_INDEX       14
#define TI_GETSIZE_INDEX    16
#define TI_GETDATAPTR_INDEX 18
#define TI_GETVATPTR_INDEX  30
#define TI_GETNAME_INDEX    31

uint24_t executeFunction(NODE*);
uint8_t parseFunction(NODE*);
//uint8_t parseFunction1Arg(uint24_t, uint8_t);
uint8_t parseFunction2Args(NODE*, NODE*, uint8_t, bool);
void LoadVariableInReg(uint8_t, uint8_t);
void LoadValueInReg(uint8_t, uint24_t, uint8_t);
uint8_t InsertDataElements(NODE*);
void loadGetKeyFastData1(void);
void loadGetKeyFastData2(void);
void InsertMallocRoutine(void);

typedef struct {
    uint8_t  function;
    uint8_t  function2;
    uint8_t  amountOfArgs;
    uint8_t  numbersArgs;
    uint8_t  pushBackwards;
} function_t;

typedef struct {
    uint8_t retRegAndArgs;
    uint8_t smallArgs;
} c_function_t;

#endif
