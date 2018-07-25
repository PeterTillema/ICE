#include "defines.h"
#include "parse.h"
#include "ast.h"
#include "operator.h"

#include "parse.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
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
        LD_BC_IMM(num, TYPE_NUMBER);
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

static element_t entry1;
static element_t entry2;
static uint24_t operand1;
static uint24_t operand2;
static uint8_t oper;
static bool canOptimizeConcatenateStrings = false;

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

uint24_t executeOperator(NODE *top) {
    uint8_t op = top->data.operand.op.op;
    uint24_t operand1 = top->child->data.operand.num;
    uint24_t operand2 = top->child->sibling->data.operand.num;
    
    switch (op) {
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

static void getEntries() {
    operand1 = entry1.operand.num;
    operand2 = entry2.operand.num;
}

static void swapEntries() {
    element_t temp;

    temp = entry1;
    entry1 = entry2;
    entry2 = temp;
    getEntries();
}

uint8_t parseOperator(NODE *top) {
    NODE *child = top->child;
    NODE *sibling = child->sibling;
    uint8_t type1, type2, res, index;
    element_t tempEntry1 = child->data, tempEntry2 = sibling->data;
    
    type1 = child->data.type;
    type2 = sibling->data.type;
    oper = top->data.operand.op.op;
    index = top->data.operand.op.index;
    
    entry1 = tempEntry1;
    entry2 = tempEntry2;
    getEntries();
    ClearAnsFlags();
    
    if (type1 == TYPE_STRING || type2 == TYPE_STRING) {
        return E_SYNTAX;
    }
    
    if (type1 > TYPE_CHAIN_PUSH) {
        if ((res = parseNode(child)) != VALID) {
            return res;
        }
        type1 = TYPE_CHAIN_ANS;
        entry1 = tempEntry1;
        entry2 = tempEntry2;
        getEntries();
    }
    
    if (type2 > TYPE_CHAIN_PUSH) {
        if (type1 == TYPE_CHAIN_ANS) {
            PushHLDE();
            expr.outputRegister = REGISTER_HL;
        }
        if ((res = parseNode(sibling)) != VALID) {
            return res;
        }
        type2 = TYPE_CHAIN_ANS;
        entry1 = tempEntry1;
        entry2 = tempEntry2;
        getEntries();
        if (type1 == TYPE_CHAIN_ANS) {
            type1 = TYPE_CHAIN_PUSH;
            (*operatorChainPushChainAnsFunctions[index - 1])();
        }
    }
    
    if (oper == tLE || oper == tLT || (operatorCanSwap[index - 1] && (type1 == TYPE_NUMBER || type2 == TYPE_CHAIN_ANS))) {
        uint8_t a = type1;
        
        type1 = type2;
        type2 = a;
        
        swapEntries();
        
        if (oper == tLE) {
            oper = tGE;
        } else if (oper == tLT) {
            oper = tGT;
        }
    }
    
    if (type1 != TYPE_CHAIN_PUSH) {
        if (type1 > TYPE_STRING) {
            type1--;
        }
        
        if (type2 > TYPE_STRING) {
            type2--;
        }
        
        // Call the right function!
        (*operatorFunctions[((index - 1) * 9) + (type1 * 3) + type2])();
        
        // If the operator is /, the routine always ends with call __idvrmu \ expr.outputReturnRegister == REGISTER_DE
        if (oper == tDiv && !(expr.outputRegister == REGISTER_A && operand2 == 1)) {
            CALL(__idvrmu);
            ResetHL();
            ResetDE();
            reg.AIsNumber = true;
            reg.AIsVariable = 0;
            reg.AValue = 0;
            expr.outputReturnRegister = REGISTER_DE;
        }

        // If the operator is *, and both operands not a number, it always ends with call __imuls
        if (oper == tMul && type1 != TYPE_NUMBER && type2 != TYPE_NUMBER && !(expr.outputRegister == REGISTER_A && operand2 < 256)) {
            CALL(__imuls);
            ResetHL();
        }

        if (expr.outputRegister != REGISTER_A && !(type2 == TYPE_NUMBER && operand2 < 256)) {
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
    
    return VALID;
}

void OperatorError(void) {
    // This *should* never be triggered
    displayError(E_ICE_ERROR);
}
void StoChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_IX_OFF_IND_HL(operand2);
    } else {
        LD_IX_OFF_IND_DE(operand2);
    }
}
void StoNumberVariable(void) {
    LD_HL_IMM(operand1, TYPE_NUMBER);
    StoChainAnsVariable();
}
void StoVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    StoChainAnsVariable();
}
void BitAndChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A) {
        if (oper == tDotIcon) {
            AND_A(operand2);
        } else if (oper == tBoxIcon) {
            XOR_A(operand2);
        } else {
            OR_A(operand2);
        }
        expr.AnsSetZeroFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
    } else {
        if (operand2 < 256) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_A_L();
            } else {
                LD_A_E();
            }
            if (oper == tDotIcon) {
                AND_A(operand2);
            } else if (oper == tBoxIcon) {
                XOR_A(operand2);
            } else {
                OR_A(operand2);
            }
            SBC_HL_HL();
            LD_L_A();
            expr.AnsSetZeroFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
        } else {
            if (expr.outputRegister != REGISTER_HL) {
                EX_DE_HL();
            }
            LD_BC_IMM(operand2, TYPE_NUMBER);
        }
    }
}
void BitAndChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(operand2);
}
void BitAndVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    BitAndChainAnsNumber();
}
void BitAndVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
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
    if (expr.outputRegister == REGISTER_A && operand2 < 256) {
        expr.outputReturnRegister = REGISTER_A;
        if (oper == tXor) {
            if (operand2) {
                OutputWrite2Bytes(OP_ADD_A, 255);
                OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
                ResetA();
            } else {
                goto NumberNotZero1;
            }
        } else if (oper == tAnd) {
            if (operand2) {
                goto NumberNotZero1;
            } else {
                LD_HL_IMM(0, TYPE_NUMBER);
                expr.outputReturnRegister = REGISTER_HL;
            }
        } else {
            if (!operand2) {
NumberNotZero1:
                OutputWrite2Bytes(OP_SUB_A, 1);
                OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
                ResetA();
            } else {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(1, TYPE_NUMBER);
                expr.outputReturnRegister = REGISTER_HL;
            }
        }
    } else {
        MaybeAToHL();
        if (oper == tXor) {
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(-1, TYPE_NUMBER);
            } else {
                LD_HL_IMM(-1, TYPE_NUMBER);
            }
            ADD_HL_DE();
            if (!operand2) {
                CCF();
                expr.AnsSetCarryFlagReversed = true;
            } else {
                expr.AnsSetCarryFlag = true;
            }
            SBC_HL_HL_INC_HL();
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3 + !operand2;
        } else if (oper == tAnd) {
            if (!operand2) {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(0, TYPE_NUMBER);
            } else {
                goto numberNotZero2;
            }
        } else {
            if (!operand2) {
numberNotZero2:
                MaybeAToHL();
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(-1, TYPE_NUMBER);
                } else {
                    LD_HL_IMM(-1, TYPE_NUMBER);
                }
                OutputWrite2Bytes(OP_ADD_HL_DE, OP_CCF);
                SBC_HL_HL_INC_HL();
                expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
                expr.AnsSetCarryFlagReversed = true;
            } else {
                ice.programPtr = ice.programPtrBackup;
                ice.dataOffsetElements = ice.dataOffsetElementsBackup;
                LD_HL_IMM(1, TYPE_NUMBER);
            }
        }
    }
}
void AndChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(operand2);
    } else {
        LD_HL_IND_IX_OFF(operand2);
    }
    AndInsert();
}
void AndVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    AndChainAnsNumber();
}
void AndVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
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
    uint24_t number = operand2;

    if (expr.outputRegister == REGISTER_A && operand2 < 256) {
        if (oper == tNE) {
            ADD_A(255 - operand2);
            OutputWrite2Bytes(OP_ADD_A, 1);
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
        } else {
            SUB_A(operand2);
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
                LD_DE_IMM(-1, TYPE_NUMBER);
            } else {
                LD_HL_IMM(-1, TYPE_NUMBER);
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
                LD_DE_IMM(number, TYPE_NUMBER);
            } else {
                LD_HL_IMM(number, TYPE_NUMBER);
            }
            EQInsert();
        }
    }
}
void EQVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    EQChainAnsNumber();
}
void EQChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(operand2);
    } else {
        LD_HL_IND_IX_OFF(operand2);
    }
    EQInsert();
}
void EQVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
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
    if (expr.outputRegister == REGISTER_A && operand2 < 256) {
        SUB_A(operand2 + (oper == tGT || oper == tLT));
        OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
        expr.AnsSetCarryFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
    } else {
        MaybeAToHL();
        if (expr.outputRegister == REGISTER_HL) {
            LD_DE_IMM(-entry2_operand - (oper == tLE || oper == tGT));
        } else {
            LD_HL_IMM(-entry2_operand - (oper == tLE || oper == tGT));
        }
        ADD_HL_DE();
        if (oper == tGE || oper == tGT) {
            CCF();
            expr.AnsSetCarryFlagReversed = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
        } else {
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
        }
        SBC_HL_HL_INC_HL();
    }
}
void GEChainAnsVariable(void) {
    AnsToHL();
    LD_DE_IND_IX_OFF(operand2);
    GEInsert();
}
void GENumberChainAns(void) {
    if (expr.outputRegister == REGISTER_A && operand1 < 256) {
        ADD_A(256 - operand1 - (oper == tGE || oper == tLE));
        OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
        expr.AnsSetCarryFlag = true;
        expr.outputReturnRegister = REGISTER_A;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
    } else {
        MaybeAToHL();
        if (expr.outputRegister == REGISTER_HL) {
            LD_DE_IMM(-entry1_operand - (oper == tGE || oper == tLT));
        } else {
            LD_HL_IMM(-entry1_operand - (oper == tGE || oper == tLT));
        }
        ADD_HL_DE();
        if (oper == tLE || oper == tLT) {
            CCF();
            expr.AnsSetCarryFlagReversed = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
        } else {
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
        }
        SBC_HL_HL_INC_HL();
    }
}
void GENumberVariable(void) {
    LD_HL_IND_IX_OFF(entry2_operand);
    GENumberChainAns();
}
void GEVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    GEChainAnsNumber();
}
void GEVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    GEChainAnsVariable();
}
void GEVariableChainAns(void) {
    AnsToDE();
    LD_HL_IND_IX_OFF(operand1);
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
    uint24_t number = operand2;

    if (expr.outputRegister == REGISTER_A && operand2 < 256) {
        LD_L_A();
        LD_H(operand2);
        MLT_HL();
    } else {
        MaybeAToHL();
        if (!number) {
            ice.programPtr = ice.programPtrBackup;
            ice.dataOffsetElements = ice.dataOffsetElementsBackup;
            LD_HL_IMM(0, TYPE_NUMBER);
        } else if (number == 0xFFFFFF) {
            CALL(__ineg);
        } else {
            MultWithNumber(number, (uint8_t*)&ice.programPtr, 16*expr.outputRegister);
        }
    }
}
void MulVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    MulChainAnsNumber();
}
void MulChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(operand2);
}
void MulVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    MulChainAnsVariable();
}
void MulChainPushChainAns(void) {
    AnsToHL();
    POP_BC();
}
void DivChainAnsNumber(void) {
    if (expr.outputRegister == REGISTER_A && operand2 <= 64 && !(operand2 & (operand2 - 1))) {
        while (operand2 != 1) {
            SRL_A();
            operand2 /= 2;
        }
        expr.outputReturnRegister = REGISTER_A;
    } else {
        AnsToHL();
        LD_BC_IMM(operand2, TYPE_NUMBER);
    }
}
void DivChainAnsVariable(void) {
    AnsToHL();
    LD_BC_IND_IX_OFF(operand2);
}
void DivNumberVariable(void) {
    LD_HL_IMM(operand1, TYPE_NUMBER);
    DivChainAnsVariable();
}
void DivNumberChainAns(void) {
    AnsToBC();
    LD_HL_IMM(operand1, TYPE_NUMBER);
}
void DivVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    DivChainAnsNumber();
}
void DivVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    DivChainAnsVariable();
}
void DivVariableChainAns(void) {
    AnsToBC();
    LD_HL_IND_IX_OFF(operand1);
}
void DivChainPushChainAns(void) {
    AnsToBC();
    POP_HL();
}
void AddChainAnsNumber(void) {
    uint24_t number = operand2;

    if (expr.outputRegister == REGISTER_A) {
        if (!number) {
            expr.outputReturnRegister = REGISTER_A;
            return;
        }
        LD_HL_IMM(number, TYPE_NUMBER);
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
                LD_DE_IMM(number, TYPE_NUMBER);
            } else {
                LD_HL_IMM(number, TYPE_NUMBER);
            }
            ADD_HL_DE();
        }
    }
}
void AddVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    AddChainAnsNumber();
}
void AddChainAnsVariable(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_DE_IND_IX_OFF(operand2);
    } else {
        LD_HL_IND_IX_OFF(operand2);
    }
    ADD_HL_DE();
}
void AddVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    if (operand1 == operand2) {
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
void SubChainAnsNumber(void) {
    uint24_t number = operand2;

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
                LD_DE_IMM(0 - number, TYPE_NUMBER);
            } else {
                LD_HL_IMM(0 - number, TYPE_NUMBER);
            }
            ADD_HL_DE();
        }
    }
}
void SubChainAnsVariable(void) {
    AnsToHL();
    LD_DE_IND_IX_OFF(operand2);
}
void SubNumberVariable(void) {
    LD_HL_IMM(operand1, TYPE_NUMBER);
    SubChainAnsVariable();
}
void SubNumberChainAns(void) {
    AnsToDE();
    LD_HL_IMM(operand1, TYPE_NUMBER);
}
void SubVariableNumber(void) {
    LD_HL_IND_IX_OFF(operand1);
    SubChainAnsNumber();
}
void SubVariableVariable(void) {
    LD_HL_IND_IX_OFF(operand1);
    SubChainAnsVariable();
}
void SubVariableChainAns(void) {
    AnsToDE();
    LD_HL_IND_IX_OFF(operand1);
}
void SubChainPushChainAns(void) {
    AnsToDE();
    POP_HL();
}

static void (*operatorChainPushChainAnsFunctions[17])(void) = {
    OperatorError,
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
    OperatorError,
    OperatorError,
    StoVariableVariable,
    OperatorError,
    OperatorError,
    StoChainAnsVariable,
    OperatorError,

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
