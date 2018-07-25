#include "defines.h"
#include "parse.h"
#include "ast.h"
#include "operator.h"

#include "parse.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"
#include "routines.h"
#include "prescan.h"

#ifndef CALCULATOR
extern const uint8_t AndData[];
extern const uint8_t OrData[];
extern const uint8_t XorData[];

static uint8_t clz(uint24_t x) {
    uint8_t n = 0;
    if (!x) {
        return 24;
    }
    while (!(x & (1 << 23))) {
        n++;
        x <<= 1;
    }
    return n;
}

void MultWithNumber(uint24_t num, uint8_t *programPtr, bool ChangeRegisters) {
    (void)programPtr;
    uint24_t bit;
    uint8_t po2 = !(num & (num - 1));

    if (24 - clz(num) + __builtin_popcount(num) - 2 * po2 < 10) {
        if(!po2) {
            if (!ChangeRegisters) {
                PUSH_HL();
                POP_DE();
                SetRegHLToRegDE();
            } else {
                PUSH_DE();
                POP_HL();
                SetRegDEToRegHL();
            }
        }
        for (bit = 1 << (22 - clz(num)); bit; bit >>= 1) {
            ADD_HL_HL();
            if(num & bit) {
                ADD_HL_DE();
            }
        }
    } else if (num < 0x100) {
        if (ChangeRegisters) {
            EX_DE_HL();
        }
        LD_A(num);
        CALL(__imul_b);
        ResetHL();
    } else {
        if (ChangeRegisters) {
            EX_DE_HL();
        }
        LD_BC_IMM(num);
        CALL(__imuls);
        ResetHL();
    }
}
#endif

static void (*operatorFunctions[272])(void);
static void (*operatorChainPushChainAnsFunctions[17])(void);
const char operators[]              = {tStore, tDotIcon, tCrossIcon, tBoxIcon, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
const uint8_t operatorPrecedence[]  = {0, 6, 8, 8, 2, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};
const uint8_t operatorPrecedence2[] = {9, 6, 8, 8, 2, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};
const uint8_t operatorCanSwap[]     = {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0}; // Used for operators which can swap the operands, i.e. A*B = B*A

static element_t *entry0;
static element_t *entry1;
static element_t *entry2;
static uint24_t entry0_operand;
static uint24_t entry1_operand;
static uint24_t entry2_operand;
static uint8_t oper;
static bool canOptimizeConcatenateStrings;

bool comparePtrToTempStrings(uint24_t addr) {
    return (addr == prescan.tempStrings[TempString1] || addr == prescan.tempStrings[TempString2]);
}

uint8_t getIndexOfOperator(uint8_t operator) {
    char *index;
    if ((index = memchr(operators, operator, sizeof operators))) {
        return index - operators + 1;
    }
    return 0;
}

uint24_t executeOperator(uint24_t operand1, uint24_t operand2, uint8_t operator) {
    switch (operator) {
        case tAdd:
            return operand1 + operand2;
        case tSub:
            return operand1 - operand2;
        case tMul:
            return operand1 * operand2;
        case tDiv:
            return operand1 / operand2;
        case tNE:
            return operand1 != operand2;
        case tGE:
            return operand1 >= operand2;
        case tLE:
            return operand1 <= operand2;
        case tGT:
            return operand1 > operand2;
        case tLT:
            return operand1 < operand2;
        case tEQ:
            return operand1 == operand2;
        case tOr:
            return operand1 || operand2;
        case tXor:
            return !operand1 != !operand2;
        case tDotIcon:
            return operand1 & operand2;
        case tCrossIcon:
            return operand1 | operand2;
        case tBoxIcon:
            return operand1 ^ operand2;
        default:
            return operand1 && operand2;
    }
}

static void getEntryOperands() {
    entry0_operand = entry0->operand.num;
    entry1_operand = entry1->operand.num;
    entry2_operand = entry2->operand.num;
}

static void swapEntries() {
    element_t *temp;

    temp = entry1;
    entry1 = entry2;
    entry2 = temp;
    getEntryOperands();
}

uint8_t parseOperator(element_t *outputPrevPrevPrev, element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr, bool canOptimizeStrings) {
    uint8_t type1, type2;
    
    type1 = outputPrevPrev->type;
    type2 = outputPrev->type;
    canOptimizeConcatenateStrings = canOptimizeStrings;

    oper = outputCurr->operand.num;

    // Get the right arguments
    entry0 = outputPrevPrevPrev;
    entry1 = outputPrevPrev;
    entry2 = outputPrev;
    getEntryOperands();

    ClearAnsFlags();
    
    if (outputPrevPrev->isString && outputPrev->isString && type2 == TYPE_NUMBER && oper == tStore) {
        StoStringString();
    } else if (outputPrevPrev->isString && outputPrev->isString && oper == tAdd) {
        AddStringString();
    } else if (outputPrevPrev->isString && type2 == TYPE_VARIABLE && oper == tStore) {
        StoStringVariable();
    } else {
        // Only call the function if both types are valid
        if ((type1 == type2 && (type1 == TYPE_NUMBER || type1 == TYPE_CHAIN_ANS)) ||
            (oper == tStore && (type2 != TYPE_VARIABLE  && !(type2 == TYPE_FUNCTION && outputPrev->operand.num == 0x010108))) ||
            (type2 == TYPE_CHAIN_PUSH) ||
            (type1 == TYPE_STRING || type2 == TYPE_STRING)
        ) {
            return E_SYNTAX;
        }

        // Store to a pointer
        if (oper == tStore && type2 == TYPE_FUNCTION) {
            type2 = TYPE_CHAIN_ANS;
            type1 = outputPrevPrevPrev->type;

            // If string->pointer, do it immediately
            if (type1 == TYPE_STRING) {
                StoStringChainAns();
                return VALID;
            }
        }

        if (type1 == TYPE_CHAIN_PUSH) {
            if (type2 != TYPE_CHAIN_ANS) {
                return E_ICE_ERROR;
            }

            // Call the right CHAIN_PUSH | CHAIN_ANS function
            (*operatorChainPushChainAnsFunctions[getIndexOfOperator(oper) - 1])();
        } else {
            // If you have something like "A or 1", the output is always 1, so we can remove the "ld hl, (ix+A)"
            ice.programPtrBackup = ice.programPtr;
            ice.dataOffsetElementsBackup = ice.dataOffsetElements;

            // Swap operands for compiler optimizations
            if (oper == tLE || oper == tLT || (operatorCanSwap[getIndexOfOperator(oper) - 1] && (type1 == TYPE_NUMBER || type2 == TYPE_CHAIN_ANS))) {
                uint8_t temp = type1;

                type1 = type2;
                type2 = temp;
                swapEntries();
                if (oper == tLE) {
                    oper = tGE;
                } else if (oper == tLT) {
                    oper = tGT;
                }
            }

            // Call the right function!
            (*operatorFunctions[((getIndexOfOperator(oper) - 1) * 9) + (type1 * 3) + type2])();
        }

        // If the operator is /, the routine always ends with call __idvrmu \ expr.outputReturnRegister == REGISTER_DE
        if (oper == tDiv && !(expr.outputRegister == REGISTER_A && entry2_operand == 1)) {
            CALL(__idvrmu);
            ResetHL();
            ResetDE();
            reg.AIsNumber = true;
            reg.AIsVariable = 0;
            reg.AValue = 0;
            expr.outputReturnRegister = REGISTER_DE;
        }

        // If the operator is *, and both operands not a number, it always ends with call __imuls
        if (oper == tMul && type1 != TYPE_NUMBER && type2 != TYPE_NUMBER && !(expr.outputRegister == REGISTER_A && entry2_operand < 256)) {
            CALL(__imuls);
            ResetHL();
        }

        if (expr.outputRegister != REGISTER_A && !(type2 == TYPE_NUMBER && entry2_operand < 256)) {
            if (oper == tDotIcon) {
                CALL(__iand);
                ResetHL();
            } else if (oper == tBoxIcon) {
                CALL(__ixor);
                ResetHL();
            } else if (oper == tCrossIcon) {
                CALL(__ior);
                ResetHL();
            }
        }

        // If the operator is -, and the second operand not a number, it always ends with or a, a \ sbc hl, de
        if (oper == tSub && type2 != TYPE_NUMBER) {
            OR_A_SBC_HL_DE();
        }
    }

    expr.outputRegister = expr.outputReturnRegister;
    return VALID;
}

void LD_HL_STRING(uint24_t stringPtr, uint8_t type) {
    if ((type == TYPE_STRING) && (stringPtr != prescan.tempStrings[TempString1] && stringPtr != prescan.tempStrings[TempString2])) {
        ProgramPtrToOffsetStack();
    }
    LD_HL_IMM(stringPtr);
}

void OperatorError(void) {
    // This *should* never be triggered
    displayError(E_ICE_ERROR);
}
void StoChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_IX_OFF_IND_HL(entry2_operand);
    } else {
        LD_IX_OFF_IND_DE(entry2_operand);
    }
}
void StoNumberVariable(void) {
    LD_HL_IMM(entry1_operand);
    StoChainAnsVariable();
}
void StoVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    StoChainAnsVariable();
}
void StoNumberChainAns(void) {
    uint8_t type = entry1->type;
    uint8_t mask = entry2->mask;
    
    if (type == TYPE_NUMBER) {
        if (mask == TYPE_MASK_U8) {
            LD_A(entry0_operand);
            LD_ADDR_A(entry1_operand);
        } else if (mask == TYPE_MASK_U16) {
            LD_HL_IMM(entry1_operand);
        } else {
            LD_HL_IMM(entry0_operand);
            LD_ADDR_HL(entry1_operand);
        }
    } else if (type == TYPE_VARIABLE) {
        LD_HL_IND_IX_OFF(entry1_operand);
    } else {
        AnsToHL();
    }
    if (type != TYPE_NUMBER) {
        if (mask == TYPE_MASK_U8) {
            LD_A(entry0_operand);
            LD_HL_A();
        } else if (mask == TYPE_MASK_U24) {
            LD_DE_IMM(entry0_operand);
            LD_HL_DE();
            expr.outputReturnRegister = REGISTER_DE;
        }
    }
    if (mask == TYPE_MASK_U16) {
        LD_DE_IMM(entry0_operand);
    }
    StoToChainAns();
}
void StoVariableChainAns(void) {
    uint8_t type = entry1->type;
    uint8_t mask = entry2->mask;

    if (type == TYPE_NUMBER) {
        if (mask == TYPE_MASK_U8) {
            LD_A_IND_IX_OFF(entry0_operand);
            LD_ADDR_A(entry1_operand);
        } else if (mask == TYPE_MASK_U16) {
            LD_HL_IND_IX_OFF(entry1_operand);
        } else {
            LD_HL_IND_IX_OFF(entry0_operand);
            LD_ADDR_HL(entry1_operand);
        }
    } else if (type == TYPE_VARIABLE) {
        LD_HL_IND_IX_OFF(entry1_operand);
    } else {
        AnsToHL();
    }
    if (type != TYPE_NUMBER) {
        if (mask == TYPE_MASK_U8) {
            LD_A_IND_IX_OFF(entry0_operand);
            LD_HL_A();
        } else if (mask == TYPE_MASK_U24) {
            LD_DE_IND_IX_OFF(entry0_operand);
            LD_HL_DE();
            expr.outputReturnRegister = REGISTER_DE;
        }
    }
    if (mask == TYPE_MASK_U16) {
        LD_DE_IND_IX_OFF(entry0_operand);
    }
    StoToChainAns();
}
void StoChainPushChainAns(void) {
    if (entry1->type == TYPE_CHAIN_ANS) {
        AnsToHL();
        POP_DE();
        if (entry2->mask == TYPE_MASK_U8) {
            OutputWrite2Bytes(OP_LD_A_E, OP_LD_HL_A);
        } else if (entry2->mask == TYPE_MASK_U24) {
            LD_HL_DE();
            expr.outputReturnRegister = REGISTER_DE;
        }
        StoToChainAns();
    }
}
void StoChainAnsChainAns(void) {
    uint8_t type = entry1->type;
    uint8_t mask = entry2->mask;

    if (type == TYPE_NUMBER) {
        if (mask == TYPE_MASK_U8) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_A_L();
            } else if (expr.outputRegister == REGISTER_DE) {
                LD_A_E();
            }
            LD_ADDR_A(entry1_operand);
        } else if (mask == TYPE_MASK_U16) {
            AnsToDE();
            LD_HL_IMM(entry1_operand);
        } else {
            MaybeAToHL();
            if (expr.outputRegister == REGISTER_HL) {
                LD_ADDR_HL(entry1_operand);
            } else {
                LD_ADDR_DE(entry1_operand);
            }
            expr.outputReturnRegister = expr.outputRegister;
        }
    } else if (type == TYPE_VARIABLE) {
        if (mask == TYPE_MASK_U8) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_A_L();
            } else if (expr.outputRegister == REGISTER_DE) {
                LD_A_E();
            }
            LD_HL_IND_IX_OFF(entry1_operand);
            LD_HL_A();
        } else {
            AnsToDE();
            LD_HL_IND_IX_OFF(entry1_operand);
            if (mask == TYPE_MASK_U24) {
                LD_HL_DE();
                expr.outputReturnRegister = REGISTER_DE;
            }
        }
    }
    StoToChainAns();
}
void StoStringChainAns(void) {
    uint8_t type = entry1->type;
    uint8_t mask = entry2->mask;

    if (type == TYPE_NUMBER) {
        LD_HL_STRING(entry0_operand, entry0->type);
        if (mask == TYPE_MASK_U24) {
            LD_ADDR_HL(entry1_operand);
        } else {
            LD_A_L();
            LD_ADDR_A(entry1_operand);
            if (mask == TYPE_MASK_U16) {
                LD_A_H();
                LD_ADDR_A(entry1_operand + 1);
            }
        }
    } else if (type == TYPE_VARIABLE) {
        LD_HL_STRING(entry0_operand, entry0->type);
        if (mask == TYPE_MASK_U24) {
            LD_IX_OFF_IND_HL(entry1_operand);
        } else {
            LD_IX_OFF_IND_L(entry1_operand);
            if (mask == TYPE_MASK_U16) {
                LD_IX_OFF_IND_H(entry1_operand + 1);
            }
        }
    } else {
        AnsToHL();
        if ((entry0->type == TYPE_STRING) && (entry0_operand != prescan.tempStrings[TempString1] && entry0_operand != prescan.tempStrings[TempString2])) {
            ProgramPtrToOffsetStack();
        }
        LD_DE_IMM(entry0_operand);
        if (type == TYPE_MASK_U24) {
            LD_HL_DE();
        } else {
            LD_HL_E();
            if (mask == TYPE_MASK_U16) {
                OutputWrite2Bytes(OP_INC_HL, OP_LD_HL_D);
                ResetHL();
            }
        }
    }
}
void StoToChainAns(void) {
    if (entry2->mask == TYPE_MASK_U8) {
        expr.outputReturnRegister = REGISTER_A;
    } else if (entry2->mask == TYPE_MASK_U16) {
        OutputWrite3Bytes(OP_LD_HL_E, OP_INC_HL, OP_LD_HL_D);
        ResetHL();
        EX_S_DE_HL();
    }
}
void StoStringString(void) {
    LD_HL_STRING(entry1_operand, entry1->type);
    PUSH_HL();
    LD_HL_IMM(entry2_operand);
    PUSH_HL();
    CALL(__strcpy);
    OutputWrite2Bytes(OP_POP_BC, OP_POP_BC);
    ResetBC();
}
void StoStringVariable(void) {
    LD_HL_STRING(entry1_operand, entry1->type);
    LD_IX_OFF_IND_HL(entry2_operand);
}
void BitAndChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A) {
        if (oper == tDotIcon) {
            AND_A(entry2_operand);
        } else if (oper == tBoxIcon) {
            XOR_A(entry2_operand);
        } else {
            OR_A(entry2_operand);
        }
        expr.AnsSetZeroFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
    } else {
        if (entry2_operand < 256) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_A_L();
            } else {
                LD_A_E();
            }
            if (oper == tDotIcon) {
                AND_A(entry2_operand);
            } else if (oper == tBoxIcon) {
                XOR_A(entry2_operand);
            } else {
                OR_A(entry2_operand);
            }
            SBC_HL_HL();
            LD_L_A();
            expr.AnsSetZeroFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
        } else {
            if (expr.outputRegister != REGISTER_HL) {
                EX_DE_HL();
            }
            LD_BC_IMM(entry2_operand);
        }
    }
}
void BitAndChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(entry2_operand);
}
void BitAndVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    BitAndChainAnsNumber();
}
void BitAndVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    BitAndChainAnsVariable();
}
void BitAndChainPushChainAns(void) {
    AnsToHL();
    POP_BC();
}

#define BitOrVariableNumber     BitAndVariableNumber
#define BitOrVariableVariable   BitAndVariableVariable
#define BitOrChainAnsNumber     BitAndChainAnsNumber
#define BitOrChainAnsVariable   BitAndChainAnsVariable
#define BitOrChainPushChainAns  BitAndChainPushChainAns

#define BitXorVariableNumber    BitAndVariableNumber
#define BitXorVariableVariable  BitAndVariableVariable
#define BitXorChainAnsNumber    BitAndChainAnsNumber
#define BitXorChainAnsVariable  BitAndChainAnsVariable
#define BitXorChainPushChainAns BitAndChainPushChainAns

void AndInsert(void) {
    if (oper == tOr) {
        memcpy(ice.programPtr, OrData, SIZEOF_OR_DATA);
        ice.programPtr += SIZEOF_OR_DATA;
    } else if (oper == tAnd) {
        memcpy(ice.programPtr, AndData, SIZEOF_AND_DATA);
        ice.programPtr += SIZEOF_AND_DATA;
    } else {
        memcpy(ice.programPtr, XorData, SIZEOF_XOR_DATA);
        ice.programPtr += SIZEOF_XOR_DATA;
    }
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}
void AndChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A && entry2_operand < 256) {
        expr.outputReturnRegister = REGISTER_A;
        if (oper == tXor) {
            if (entry2_operand) {
                OutputWrite2Bytes(OP_ADD_A, 255);
                OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
                ResetA();
            } else {
                goto NumberNotZero1;
            }
        } else if (oper == tAnd) {
            if (entry2_operand) {
                goto NumberNotZero1;
            } else {
                LD_HL_IMM(0);
                expr.outputReturnRegister = REGISTER_HL;
            }
        } else {
            if (!entry2_operand) {
NumberNotZero1:
                OutputWrite2Bytes(OP_SUB_A, 1);
                OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
                ResetA();
            } else {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(1);
                expr.outputReturnRegister = REGISTER_HL;
            }
        }
    } else {
        MaybeAToHL();
        if (oper == tXor) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(-1);
            } else {
                LD_HL_IMM(-1);
            }
            ADD_HL_DE();
            if (!entry2_operand) {
                CCF();
                expr.AnsSetCarryFlagReversed = true;
            } else {
                expr.AnsSetCarryFlag = true;
            }
            SBC_HL_HL_INC_HL();
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3 + !entry2_operand;
        } else if (oper == tAnd) {
            if (!entry2_operand) {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(0);
            } else {
                goto numberNotZero2;
            }
        } else {
            if (!entry2_operand) {
numberNotZero2:
                MaybeAToHL();
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(-1);
                } else {
                    LD_HL_IMM(-1);
                }
                OutputWrite2Bytes(OP_ADD_HL_DE, OP_CCF);
                SBC_HL_HL_INC_HL();
                expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
                expr.AnsSetCarryFlagReversed = true;
            } else {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(1);
            }
        }
    }
}
void AndChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    AndInsert();
}
void AndVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AndChainAnsNumber();
}
void AndVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AndChainAnsVariable();
}
void AndChainPushChainAns(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        POP_DE();
    } else {
        POP_HL();
    }
    AndInsert();
}

#define XorVariableNumber    AndVariableNumber
#define XorVariableVariable  AndVariableVariable
#define XorChainAnsNumber    AndChainAnsNumber
#define XorChainAnsVariable  AndChainAnsVariable
#define XorChainPushChainAns AndChainPushChainAns

#define OrVariableNumber     AndVariableNumber
#define OrVariableVariable   AndVariableVariable
#define OrChainAnsNumber     AndChainAnsNumber
#define OrChainAnsVariable   AndChainAnsVariable
#define OrChainPushChainAns  AndChainPushChainAns

void EQInsert() {
    OR_A_SBC_HL_DE();
    OutputWriteByte(OP_LD_HL);
    OutputWriteLong(0);
    if (oper == tEQ) {
        OutputWrite2Bytes(OP_JR_NZ, 1);
        expr.AnsSetZeroFlagReversed = true;
    } else {
        OutputWrite2Bytes(OP_JR_Z, 1);
        expr.AnsSetZeroFlag = true;
    }
    INC_HL();
    expr.ZeroCarryFlagRemoveAmountOfBytes = 7;
}
void EQChainAnsNumber(void) {
    uint24_t number = entry2_operand;

    if (expr.outputRegister == REGISTER_A && entry2_operand < 256) {
        if (oper == tNE) {
            ADD_A(255 - entry2_operand);
            OutputWrite2Bytes(OP_ADD_A, 1);
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
        } else {
            SUB_A(entry2_operand);
            OutputWrite2Bytes(OP_ADD_A, 255);
            expr.AnsSetZeroFlagReversed = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
        }
        OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
        expr.outputReturnRegister = REGISTER_A;
    } else {
        MaybeAToHL();
        if (number && number < 6) {
            do {
                if (expr.outputRegister == REGISTER_HL) {
                    DEC_HL();
                } else {
                    DEC_DE();
                }
            } while (--number);
        }
        if (!number) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(-1);
            } else {
                LD_HL_IMM(-1);
            }
            ADD_HL_DE();
            expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
            if (oper == tNE) {
                CCF();
                expr.ZeroCarryFlagRemoveAmountOfBytes++;
                expr.AnsSetCarryFlagReversed = true;
            } else {
                expr.AnsSetCarryFlag = true;
            }
            SBC_HL_HL_INC_HL();
            expr.ZeroCarryFlagRemoveAmountOfBytes += 3;
        } else {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(number);
            } else {
                LD_HL_IMM(number);
            }
            EQInsert();
        }
    }
}
void EQVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    EQChainAnsNumber();
}
void EQChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    EQInsert();
}
void EQVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    EQChainAnsVariable();
}
void EQChainPushChainAns(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        POP_DE();
    } else {
        POP_HL();
    }
    EQInsert();
}
void GEInsert() {
    if (oper == tGE || oper == tLE) {
        OR_A_A();
    } else {
        SCF();
    }
    SBC_HL_DE();
    SBC_HL_HL_INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}
void GEChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A && entry2_operand < 256) {
        SUB_A(entry2_operand + (oper == tGT || oper == tLT));
        OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
        expr.AnsSetCarryFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
    } else {
        AnsToHL();
        LD_DE_IMM(entry2_operand);
        GEInsert();
    }
}
void GEChainAnsVariable(void) {
    AnsToHL();
    LD_DE_IND_IX_OFF(entry2_operand);
    GEInsert();
}
void GENumberVariable(void) {
    LD_HL_IMM(entry1_operand);
    GEChainAnsVariable();
}
void GENumberChainAns(void) {
    if (expr.outputRegister == REGISTER_A && entry1_operand < 256) {
        ADD_A(256 - entry1_operand - (oper == tGE || oper == tLE));
        OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
        expr.AnsSetCarryFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
    } else {
        AnsToDE();
        LD_HL_IMM(entry1_operand);
        GEInsert();
    }
}
void GEVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    GEChainAnsNumber();
}
void GEVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    GEChainAnsVariable();
}
void GEVariableChainAns(void) {
    AnsToDE();
    LD_HL_IND_IX_OFF(entry1_operand);
    GEInsert();
}
void GEChainPushChainAns(void) {
    AnsToDE();
    POP_HL();
    GEInsert();
}

#define GTNumberVariable    GENumberVariable
#define GTNumberChainAns    GENumberChainAns
#define GTVariableNumber    GEVariableNumber
#define GTVariableVariable  GEVariableVariable
#define GTVariableChainAns  GEVariableChainAns
#define GTChainAnsNumber    GEChainAnsNumber
#define GTChainAnsVariable  GEChainAnsVariable
#define GTChainPushChainAns GEChainPushChainAns

#define LTNumberVariable   GTVariableNumber
#define LTNumberChainAns   GTChainAnsNumber
#define LTVariableNumber   GTNumberVariable
#define LTVariableVariable GTVariableVariable
#define LTVariableChainAns GTChainAnsVariable
#define LTChainAnsNumber   GTNumberChainAns
#define LTChainAnsVariable GTVariableChainAns

void LTChainPushChainAns(void) {
    AnsToHL();
    POP_DE();
    SCF();
    SBC_HL_DE();
    SBC_HL_HL_INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}

#define LENumberVariable   GEVariableNumber
#define LENumberChainAns   GEChainAnsNumber
#define LEVariableNumber   GENumberVariable
#define LEVariableVariable GEVariableVariable
#define LEVariableChainAns GEChainAnsVariable
#define LEChainAnsNumber   GENumberChainAns
#define LEChainAnsVariable GEVariableChainAns

void LEChainPushChainAns(void) {
    AnsToHL();
    POP_DE();
    OR_A_SBC_HL_DE();
    SBC_HL_HL_INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}

#define NEVariableNumber    EQVariableNumber
#define NEVariableVariable  EQVariableVariable
#define NEChainAnsNumber    EQChainAnsNumber
#define NEChainAnsVariable  EQChainAnsVariable
#define NEChainPushChainAns EQChainPushChainAns

void MulChainAnsNumber(void) {
    uint24_t number = entry2_operand;

    if (expr.outputRegister == REGISTER_A && entry2_operand < 256) {
        LD_L_A();
        LD_H(entry2_operand);
        MLT_HL();
    } else {
        MaybeAToHL();
        if (!number) {
            ice.programPtr = ice.programPtrBackup;
            ice.dataOffsetElements = ice.dataOffsetElementsBackup;
            LD_HL_IMM(0);
        } else if (number == 0xFFFFFF) {
            CALL(__ineg);
        } else {
            MultWithNumber(number, (uint8_t*)&ice.programPtr, 16*expr.outputRegister);
        }
    }
}
void MulVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    MulChainAnsNumber();
}
void MulChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(entry2_operand);
}
void MulVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    MulChainAnsVariable();
}
void MulChainPushChainAns(void) {
    AnsToHL();
    POP_BC();
}
void DivChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A && entry2_operand <= 64 && !(entry2_operand & (entry2_operand - 1))) {
        while (entry2_operand != 1) {
            SRL_A();
            entry2_operand /= 2;
        }
        expr.outputReturnRegister = REGISTER_A;
    } else {
        AnsToHL();
        LD_BC_IMM(entry2_operand);
    }
}
void DivChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(entry2_operand);
}
void DivNumberVariable(void) {
    LD_HL_IMM(entry1_operand);
    DivChainAnsVariable();
}
void DivNumberChainAns(void) {
    AnsToBC();
    LD_HL_IMM(entry1_operand);
}
void DivVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    DivChainAnsNumber();
}
void DivVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    DivChainAnsVariable();
}
void DivVariableChainAns(void) {
    AnsToBC();
    LD_HL_IND_IX_OFF(entry1_operand);
}
void DivChainPushChainAns(void) {
    AnsToBC();
    POP_HL();
}
void AddChainAnsNumber(void) {
    uint24_t number = entry2_operand;

    if (expr.outputRegister == REGISTER_A) {
        if (!number) {
            expr.outputReturnRegister = REGISTER_A;
            return;
        }
        LD_HL_IMM(number);
        OutputWrite2Bytes(OP_ADD_A_L, OP_LD_L_A);
        if ((number & 0x00FF00) != 0x00FF00) {
            OutputWrite3Bytes(OP_ADC_A_H, OP_SUB_A_L, OP_LD_H_A);
        } else {
            OutputWrite3Bytes(OP_JR_NC, 4, OP_LD_L);
            OutputWrite3Bytes(-1, OP_INC_HL, OP_LD_L_A);
        }
        ResetHL();
    } else {
        if (number < 5) {
            uint8_t a;

            for (a = 0; a < (uint8_t)number; a++) {
                if (expr.outputRegister == REGISTER_HL) {
                    INC_HL();
                } else {
                    INC_DE();
                }
            }
            expr.outputReturnRegister = expr.outputRegister;
        } else {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(number);
            } else {
                LD_HL_IMM(number);
            }
            ADD_HL_DE();
        }
    }
}
void AddVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AddChainAnsNumber();
}
void AddChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    ADD_HL_DE();
}
void AddVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    if (entry1_operand == entry2_operand) {
        ADD_HL_HL();
    } else {
        AddChainAnsVariable();
    }
}
void AddChainPushChainAns(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        POP_DE();
    } else {
        POP_HL();
    }
    ADD_HL_DE();
}
void AddStringString(void) {
    /**
    *    Cases:
    *        <string1>+<string2>             --> strcpy(<TempString1>, <string1>) strcat(<TempString1>, <string2>)
    *        <string1>+<TempString1>         --> strcpy(<TempString2>, <string1>) strcat(<TempString2>, <TempString1>)
    *        <string1>+<TempString2>         --> strcpy(<TempString1>, <string1>) strcat(<TempString1>, <TempString2>)
    *        <TempString1>+<string2>         --> strcat(<TempString1>, <string2>)
    *        <TempString1>+<TempString2>     --> strcat(<TempString1>, <TempString2>)
    *        <TempString2>+<string2>         --> strcat(<TempString2>, <string2>)
    *        <TempString2>+<TempString1>     --> strcat(<TempString2>, <TempString1>)
    *
    *    Output in TempString2 if:
    *        <TempString2>+X
    *        X+<TempString1>
    **/

    if (comparePtrToTempStrings(entry1_operand)) {
        if (entry2->type == TYPE_STRING && !comparePtrToTempStrings(entry2_operand)) {
            ProgramPtrToOffsetStack();
        }
        LD_HL_IMM(entry2_operand);
        PUSH_HL();
        LD_HL_IMM(entry1_operand);
        PUSH_HL();
        CALL(__strcat);
        OutputWrite2Bytes(OP_POP_BC, OP_POP_BC);
        ResetBC();
    } else {
        // Optimize StrX + "..." -> StrX
        if (canOptimizeConcatenateStrings) {
            if (entry2->type == TYPE_STRING && !comparePtrToTempStrings(entry2_operand)) {
                ProgramPtrToOffsetStack();
            }
            LD_HL_IMM(entry2_operand);
            PUSH_HL();
            LD_HL_IMM(entry1_operand);
            PUSH_HL();
        } else {
            if (entry1->type == TYPE_STRING && !comparePtrToTempStrings(entry1_operand)) {
                ProgramPtrToOffsetStack();
            }
            LD_HL_IMM(entry1_operand);
            PUSH_HL();
            if (entry2_operand == prescan.tempStrings[TempString1]) {
                LD_HL_IMM(prescan.tempStrings[TempString2]);
            } else {
                LD_HL_IMM(prescan.tempStrings[TempString1]);
            }
            PUSH_HL();
            CALL(__strcpy);
            POP_DE();
            if (entry2->type == TYPE_STRING && !comparePtrToTempStrings(entry2_operand)) {
                ProgramPtrToOffsetStack();
            }
            LD_HL_IMM(entry2_operand);
            EX_SP_HL();
            PUSH_DE();
        }
        CALL(__strcat);
        OutputWrite2Bytes(OP_POP_BC, OP_POP_BC);
        ResetBC();
    }
}
void SubChainAnsNumber(void) {
    uint24_t number = entry2_operand;

    if (expr.outputRegister == REGISTER_A && number < 256) {
        if (!number) {
            expr.outputReturnRegister = REGISTER_A;
            return;
        }
        SUB_A(number);
        SBC_HL_HL();
        LD_L_A();
        expr.AnsSetZeroFlag = true;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
    } else {
        MaybeAToHL();

        if (number < 5) {
            uint8_t a;

            for (a = 0; a < (uint8_t)number; a++) {
                if (expr.outputRegister == REGISTER_HL) {
                    DEC_HL();
                } else {
                    DEC_DE();
                }
                expr.outputReturnRegister = expr.outputRegister;
            }
        } else {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(0 - number);
            } else {
                LD_HL_IMM(0 - number);
            }
            ADD_HL_DE();
        }
    }
}
void SubChainAnsVariable(void) {
    AnsToHL();
    LD_DE_IND_IX_OFF(entry2_operand);
}
void SubNumberVariable(void) {
    LD_HL_IMM(entry1_operand);
    SubChainAnsVariable();
}
void SubNumberChainAns(void) {
    AnsToDE();
    LD_HL_IMM(entry1_operand);
}
void SubVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    SubChainAnsNumber();
}
void SubVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    SubChainAnsVariable();
}
void SubVariableChainAns(void) {
    AnsToDE();
    LD_HL_IND_IX_OFF(entry1_operand);
}
void SubChainPushChainAns(void) {
    AnsToDE();
    POP_HL();
}

static void (*operatorChainPushChainAnsFunctions[17])(void) = {
    StoChainPushChainAns,
    BitAndChainPushChainAns,
    BitOrChainPushChainAns,
    BitXorChainPushChainAns,
    AndChainPushChainAns,
    XorChainPushChainAns,
    OrChainPushChainAns,
    EQChainPushChainAns,
    LTChainPushChainAns,
    GTChainPushChainAns,
    LEChainPushChainAns,
    GEChainPushChainAns,
    NEChainPushChainAns,
    MulChainPushChainAns,
    DivChainPushChainAns,
    AddChainPushChainAns,
    SubChainPushChainAns
};

static void (*operatorFunctions[272])(void) = {
    OperatorError,
    StoNumberVariable,
    StoNumberChainAns,
    OperatorError,
    StoVariableVariable,
    StoVariableChainAns,
    OperatorError,
    StoChainAnsVariable,
    StoChainAnsChainAns,

    OperatorError,
    OperatorError,
    OperatorError,
    BitAndVariableNumber,
    BitAndVariableVariable,
    OperatorError,
    BitAndChainAnsNumber,
    BitAndChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    BitOrVariableNumber,
    BitOrVariableVariable,
    OperatorError,
    BitOrChainAnsNumber,
    BitOrChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    BitXorVariableNumber,
    BitXorVariableVariable,
    OperatorError,
    BitXorChainAnsNumber,
    BitXorChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    AndVariableNumber,
    AndVariableVariable,
    OperatorError,
    AndChainAnsNumber,
    AndChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    XorVariableNumber,
    XorVariableVariable,
    OperatorError,
    XorChainAnsNumber,
    XorChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    OrVariableNumber,
    OrVariableVariable,
    OperatorError,
    OrChainAnsNumber,
    OrChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    EQVariableNumber,
    EQVariableVariable,
    OperatorError,
    EQChainAnsNumber,
    EQChainAnsVariable,
    OperatorError,

    OperatorError,
    LTNumberVariable,
    LTNumberChainAns,
    LTVariableNumber,
    LTVariableVariable,
    LTVariableChainAns,
    LTChainAnsNumber,
    LTChainAnsVariable,
    OperatorError,

    OperatorError,
    GTNumberVariable,
    GTNumberChainAns,
    GTVariableNumber,
    GTVariableVariable,
    GTVariableChainAns,
    GTChainAnsNumber,
    GTChainAnsVariable,
    OperatorError,

    OperatorError,
    LENumberVariable,
    LENumberChainAns,
    LEVariableNumber,
    LEVariableVariable,
    LEVariableChainAns,
    LEChainAnsNumber,
    LEChainAnsVariable,
    OperatorError,

    OperatorError,
    GENumberVariable,
    GENumberChainAns,
    GEVariableNumber,
    GEVariableVariable,
    GEVariableChainAns,
    GEChainAnsNumber,
    GEChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    NEVariableNumber,
    NEVariableVariable,
    OperatorError,
    NEChainAnsNumber,
    NEChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    MulVariableNumber,
    MulVariableVariable,
    OperatorError,
    MulChainAnsNumber,
    MulChainAnsVariable,
    OperatorError,

    OperatorError,
    DivNumberVariable,
    DivNumberChainAns,
    DivVariableNumber,
    DivVariableVariable,
    DivVariableChainAns,
    DivChainAnsNumber,
    DivChainAnsVariable,
    OperatorError,

    OperatorError,
    OperatorError,
    OperatorError,
    AddVariableNumber,
    AddVariableVariable,
    OperatorError,
    AddChainAnsNumber,
    AddChainAnsVariable,
    OperatorError,

    OperatorError,
    SubNumberVariable,
    SubNumberChainAns,
    SubVariableNumber,
    SubVariableVariable,
    SubVariableChainAns,
    SubChainAnsNumber,
    SubChainAnsVariable,
    OperatorError,
};
