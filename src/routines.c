#include "defines.h"
#include "main.h"
#include "routines.h"

#include "ast.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"

#define LB_X 80
#define LB_Y 210
#define LB_W 160
#define LB_H 10

extern prescan_t prescan;
extern const uint8_t implementedFunctions[AMOUNT_OF_FUNCTIONS][5];

uint8_t GetIndexOfFunction(uint8_t tok1, uint8_t tok2) {
    uint8_t a;
    
    if (!IsA2ByteTok(tok1)) {
        tok2 = 0;
    }
    
    for (a = 0; a < AMOUNT_OF_FUNCTIONS; a++) {
        if (implementedFunctions[a][0] == tok1 && implementedFunctions[a][1] == tok2) {
            return a;
        }
    }
    
    return -1;
}

void OutputWriteByte(uint8_t byte) {
    *ice.programPtr++ = byte;
}

void OutputWriteWord(uint16_t val) {
#ifdef CALCULATOR
    *(uint16_t *)ice.programPtr = val;
#else
    w16(ice.programPtr, val);
#endif
    ice.programPtr += 2;
}

void OutputWriteLong(uint24_t val) {
#ifdef CALCULATOR
    *(uint24_t *)ice.programPtr = val;
#else
    w24(ice.programPtr, val);
 #endif
    ice.programPtr += 3;
}

void OutputWriteMem(uint8_t mem[]) {
    uint24_t size = strlen((char*)mem);
    
    memcpy(ice.programPtr, mem, size);
    ice.programPtr += size;
}

bool IsA2ByteTok(uint8_t tok) {
    uint8_t All2ByteTokens[9] = {tExtTok, tVarMat, tVarLst, tVarPict, tVarGDB, tVarOut, tVarSys, tVarStrng, t2ByteTok};
    
    return memchr(All2ByteTokens, tok, sizeof(All2ByteTokens)) || 0;
}

prog_t *GetProgramName(void) {
    prog_t *ret;
    uint8_t a = 0;
    int token;
    
    ret = (prog_t*)malloc(sizeof(prog_t));
    ret->errorCode = VALID;

    while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tRParen) {
        if (a == 8) {
            ret->errorCode = E_INVALID_PROG;
            return ret;
        }
        ret->prog[a++] = (uint8_t)token;
    }
    if (!a) {
        ret->errorCode = E_INVALID_PROG;
    }
    
    ret->prog[a] = 0;

    return ret;
}

void ProgramPtrToOffsetStack(void) {
    ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.programPtr + 1);
    reg.allowedToOptimize = false;
}

void AnsToHL(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_DE) {
        EX_DE_HL();
    }
    expr.outputRegister = REGISTER_HL;
}

void AnsToDE(void) {
    if (expr.outputRegister == REGISTER_HL) {
        EX_DE_HL();
    } else if (expr.outputRegister == REGISTER_A) {
        LD_DE_IMM(0);
        LD_E_A();
        reg.DEIsNumber = reg.AIsNumber;
        reg.DEIsVariable = false;
        reg.DEValue = reg.AValue;
    }
    expr.outputRegister = REGISTER_DE;
}

void AnsToBC(void) {
    if (expr.outputRegister == REGISTER_A) {
        LD_BC_IMM(0);
        LD_C_A();
        reg.BCIsNumber = reg.AIsNumber;
        reg.BCIsVariable = reg.AIsVariable;
        reg.BCValue = reg.AValue;
    } else {
        PushHLDE();
        POP_BC();
    }
}

void AnsToA(void) {
    if (expr.outputRegister == REGISTER_HL) {
        LD_A_L();
    } else if (expr.outputRegister == REGISTER_DE) {
        LD_A_E();
    }
    expr.outputRegister = REGISTER_A;
}

void SetRegHLToRegDE(void) {
    reg.DEIsNumber = reg.HLIsNumber;
    reg.DEIsVariable = reg.HLIsVariable;
    reg.DEValue = reg.HLValue;
    reg.DEVariable = reg.HLVariable;
}

void SetRegDEToRegHL(void) {
    reg.HLIsNumber = reg.DEIsNumber;
    reg.HLIsVariable = reg.DEIsVariable;
    reg.HLValue = reg.DEValue;
    reg.HLVariable = reg.DEVariable;
}

void ClearAnsFlags(void) {
    expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed = expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed = false;
}

void ChangeRegValue(uint24_t inValue, uint24_t outValue, uint8_t opcodes[7]) {
    expr.SizeOfOutputNumber = 0;

    if (reg.allowedToOptimize) {
        uint8_t a;
        if (outValue - inValue < 5) {
            for (a = 0; a < (uint8_t)(outValue - inValue); a++) {
                OutputWriteByte(opcodes[0]);
                expr.SizeOfOutputNumber++;
            }
        } else if (inValue - outValue < 5) {
            for (a = 0; a < (uint8_t)(inValue - outValue); a++) {
                OutputWriteByte(opcodes[1]);
                expr.SizeOfOutputNumber++;
            }
        } else if (inValue < 256 && outValue < 512) {
            if (outValue > 255) {
                OutputWriteByte(opcodes[2]);
                expr.SizeOfOutputNumber = 1;
            }
            if (inValue != (outValue & 255)) {
                OutputWriteByte(opcodes[4]);
                OutputWriteByte(outValue);
                expr.SizeOfOutputNumber += 2;
            }
        } else if (inValue < 512 && outValue < 256) {
            OutputWriteByte(opcodes[3]);
            expr.SizeOfOutputNumber = 1;
            if ((inValue & 255) != outValue) {
                OutputWriteByte(opcodes[4]);
                OutputWriteByte(outValue);
                expr.SizeOfOutputNumber = 3;
            }
        } else if (inValue < 65536 && outValue < 65536 && (inValue & 255) == (outValue & 255)) {
            OutputWriteByte(opcodes[5]);
            OutputWriteByte(outValue >> 8);
            expr.SizeOfOutputNumber = 2;
        } else if (outValue >= IX_VARIABLES - 0x80 && outValue <= IX_VARIABLES + 0x7F) {
            OutputWrite2Bytes(0xED, 0x22);
            OutputWriteByte(outValue - IX_VARIABLES);
            expr.SizeOfOutputNumber = 3;
        } else {
            OutputWriteByte(opcodes[6]);
            OutputWriteLong(outValue);
            expr.SizeOfOutputNumber = 4;
        }
    } else {
        OutputWriteByte(opcodes[6]);
        OutputWriteLong(outValue);
        expr.SizeOfOutputNumber = 4;
    }
    reg.allowedToOptimize = true;
}

void LoadRegValue(uint8_t reg2, uint24_t val) {
    if (reg2 == REGISTER_HL) {
        if (reg.HLIsNumber) {
            uint8_t opcodes[7] = {OP_INC_HL, OP_DEC_HL, OP_INC_H, OP_DEC_H, OP_LD_L, OP_LD_H, OP_LD_HL};

            ChangeRegValue(reg.HLValue, val, opcodes);
        } else if (val) {
            OutputWriteByte(OP_LD_HL);
            OutputWriteLong(val);
            expr.SizeOfOutputNumber = 4;
        } else {
            OR_A_A();
            SBC_HL_HL();
            expr.SizeOfOutputNumber = 3;
        }
        reg.HLIsNumber = true;
        reg.HLIsVariable = false;
        reg.HLValue = val;
    } else if (reg2 == REGISTER_DE) {
        if (reg.DEIsNumber) {
            uint8_t opcodes[7] = {OP_INC_DE, OP_DEC_DE, OP_INC_D, OP_DEC_D, OP_LD_E, OP_LD_D, OP_LD_DE};

            ChangeRegValue(reg.DEValue, val, opcodes);
        } else {
            OutputWriteByte(OP_LD_DE);
            OutputWriteLong(val);
            expr.SizeOfOutputNumber = 4;
        }
        reg.DEIsNumber = true;
        reg.DEIsVariable = false;
        reg.DEValue = val;
    } else {
        reg.BCIsNumber = true;
        reg.BCIsVariable = false;
        reg.BCValue = val;
        OutputWriteByte(OP_LD_BC);
        OutputWriteLong(val);
        expr.SizeOfOutputNumber = 4;
    }
}

void LoadRegVariable(uint8_t reg2, uint8_t variable) {
    if (reg2 == REGISTER_HL) {
        if (!(reg.HLIsVariable && reg.HLVariable == variable)) {
            OutputWrite2Bytes(0xDD, 0x27);
            OutputWriteByte(variable);
            reg.HLIsNumber = false;
            reg.HLIsVariable = true;
            reg.HLVariable = variable;
        }
    } else if (reg2 == REGISTER_DE) {
        if (!(reg.DEIsVariable && reg.DEVariable == variable)) {
            OutputWrite2Bytes(0xDD, 0x17);
            OutputWriteByte(variable);
            reg.DEIsNumber = false;
            reg.DEIsVariable = true;
            reg.DEVariable = variable;
        }
    } else if (reg2 == REGISTER_BC) {
        reg.BCIsNumber = false;
        reg.BCIsVariable = true;
        reg.BCVariable = variable;
        OutputWrite2Bytes(0xDD, 0x07);
        OutputWriteByte(variable);
    } else {
        reg.AIsNumber = false;
        reg.AIsVariable = true;
        reg.AVariable = variable;
        OutputWrite2Bytes(0xDD, 0x7E);
        OutputWriteByte(variable);
    }
}

void ResetAllRegs(void) {
    reg.HLIsNumber = reg.HLIsVariable = reg.DEIsNumber = reg.DEIsVariable = reg.BCIsNumber = reg.BCIsVariable = reg.AIsNumber = reg.AIsVariable = false;
}

void ResetHL(void) {
    reg.HLIsNumber = reg.HLIsVariable = false;
}

void ResetDE(void) {
    reg.DEIsNumber = reg.DEIsVariable = false;
}

void ResetBC(void) {
    reg.BCIsNumber = reg.BCIsVariable = false;
}

void ResetA(void) {
    reg.AIsNumber = reg.AIsVariable = false;
}

void RegChangeHLDE(void) {
    uint8_t  temp8;
    uint24_t temp24;

    temp8 = reg.HLIsNumber;
    reg.HLIsNumber = reg.DEIsNumber;
    reg.DEIsNumber = temp8;
    temp24 = reg.HLValue;
    reg.HLValue = reg.DEValue;
    reg.DEValue = temp24;

    temp8 = reg.HLIsVariable;
    reg.HLIsVariable = reg.DEIsVariable;
    reg.DEIsVariable = temp8;
    temp8 = reg.HLVariable;
    reg.HLVariable = reg.DEVariable;
    reg.DEVariable = temp8;
}

void MaybeAToHL(void) {
    if (expr.outputRegister == REGISTER_A) {
        OR_A_A();
        SBC_HL_HL();
        LD_L_A();
        reg.HLIsNumber = reg.AIsNumber;
        reg.HLIsVariable = false;
        reg.HLValue = reg.AValue;
        expr.outputRegister = REGISTER_HL;
    }
}

void SeekMinus1(void) {
    _seek(-1, SEEK_CUR, ice.inPrgm);
}

void displayMessageLineScroll(char *string) {
#ifdef CALCULATOR
    char c;

    gfx_SetTextXY(1, gfx_GetTextY());

    // Display the string
    while(c = *string++) {
        if (gfx_GetTextY() > 190) {
            gfx_SetClipRegion(0, 11, 320, LB_Y - 1);
            gfx_ShiftUp(10);
            gfx_SetColor(255);
            gfx_SetClipRegion(0, 0, 320, 240);
            gfx_FillRectangle(0, LB_Y - 11, 320, 10);
            gfx_SetTextXY(1, gfx_GetTextY() - 10);
        }
        if (c == tTheta) {
            c = 0x40;
        }
        gfx_PrintChar(c);
        if (gfx_GetTextX() > 312) {
            gfx_SetTextXY(1, gfx_GetTextY() + 10);
        }
    }
    gfx_SetTextXY(1, gfx_GetTextY() + 10);
#else
    fprintf(stdout, "%s\n", string);
#endif
}

void MaybeLDIYFlags(void) {
    if (ice.modifiedIY) {
        LD_IY_IMM(flags);
        ice.modifiedIY = false;
    }
}

void PushHLDE(void) {
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
}

uint8_t IsHexadecimal(int token) {
    uint8_t tok = token;

    if (tok >= t0 && tok <= t9) {
        return tok - t0;
    } else if (tok >= tA && tok <= tF) {
        return tok - tA + 10;
    } else {
        return 16;
    }
}

bool CheckEOL(void) {
    int token;

    if ((token = _getc()) == EOF || (uint8_t)token == tEnter || (uint8_t)token == tColon) {
        return true;
    }
    return false;
}

void CallRoutine(bool *routineBool, uint24_t *routineAddress, const uint8_t *routineData, uint8_t routineLength) {
    // Store the pointer to the call to the stack, to replace later
    ProgramPtrToOffsetStack();

    // We need to add the routine to the data section
    if (!*routineBool) {
        ice.programDataPtr -= routineLength;
        *routineAddress = (uintptr_t)ice.programDataPtr;
        memcpy(ice.programDataPtr, routineData, routineLength);
        *routineBool = true;
    }

    CALL(*routineAddress);
}

uint8_t GetVariableOffset(uint8_t tok) {
    char variableName[21] = {0};
    variable_t *variableNew;
    uint8_t a = 1, b;

    variableName[0] = tok;
    while ((tok = _getc()) >= tA && tok <= tTheta && a < 20) {
        variableName[a++] = tok;
    }
    variableName[a] = 0;
    if (tok != 0xFF) {
        SeekMinus1();
    }

    // This variable already exists
    for (b = 0; b < prescan.amountOfVariablesUsed; b++) {
        if (!strcmp(variableName, (&prescan.variables[b])->name)) {
            return b * 3 - 128;
        }
    }

    // Create new variable
    variableNew = &prescan.variables[prescan.amountOfVariablesUsed];
    memcpy(variableNew->name, variableName, a + 1);

    return prescan.amountOfVariablesUsed++ * 3 - 128;
}

#ifdef CALCULATOR

void displayLoadingBarFrame(void) {
    // Display a fancy loading bar during compiling ;)
    gfx_SetColor(0);
    gfx_Rectangle_NoClip(LB_X, LB_Y, LB_W, LB_H);
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(LB_X + 1, LB_Y + 1, LB_W - 2, LB_H - 2);
}

void displayLoadingBar(void) {
    gfx_SetColor(4);
    gfx_FillRectangle_NoClip(LB_X + 1, LB_Y + 1, ti_Tell(ice.inPrgm) * (LB_W - 2) / ti_GetSize(ice.inPrgm), LB_H - 2);
}

int getNextToken(void) {
    return ti_GetC(ice.inPrgm);
}

int grabString(uint8_t **outputPtr, bool stopAtStoreAndString) {
    void *dataPtr = ti_GetDataPtr(ice.inPrgm);
    uint24_t token;

    while ((token = _getc()) != EOF && !(stopAtStoreAndString && ((uint8_t)token == tString || (uint8_t)token == tStore)) && (uint8_t)token != tEnter) {
        uint24_t strLength, a;
        const char *dataString;
        uint8_t tokSize;

        // Get the token in characters, and copy to the output
        dataString = ti_GetTokenString(&dataPtr, &tokSize, &strLength);
        memcpy(*outputPtr, dataString, strLength);

        for (a = 0; a < strLength; a++) {
            uint8_t char2 = *(*outputPtr + a);

            // Differences in TI-ASCII and ASCII set, C functions expect the normal ASCII set
            // There are no 4sqrt( and theta symbol in the first 128 characters of the ASCII set
            if (char2 == 0x24 || char2 == 0x5B) {
                char2 = 0;
            }

            // $ = 0x24
            if (char2 == 0xF2) {
                char2 = 0x24;
            }

            // [ = 0x5B
            if (char2 == 0xC1) {
                char2 = 0x5B;
            }

            // All the first 32 and last 128 characters are different
            if (char2 < 32 || char2 > 127) {
                displayError(W_WRONG_CHAR);
                char2 = 0;
            }

            *(*outputPtr + a) = char2;
        }
        
        *outputPtr += strLength;

        // If it's a 2-byte token, we also need to get the second byte of it
        if (tokSize == 2) {
            _getc();
        }
    }

    return token;
}

void printButton(uint24_t xPos) {
    gfx_SetColor(0);
    gfx_Rectangle_NoClip(xPos, 230, 40, 11);
    gfx_SetPixel(xPos + 1, 231);
    gfx_SetPixel(xPos + 38, 231);
    gfx_SetColor(255);
    gfx_SetPixel(xPos, 230);
    gfx_SetPixel(xPos + 39, 230);
}

#else

const char *tokenStrings[] = {
    "►DMS",
    "►Dec",
    "►Frac",
    "→",
    "Boxplot",
    "[",
    "]",
    "{",
    "}",
    "ʳ",
    "°",
    "ˉ¹",
    "²",
    "ᵀ",
    "³",
    "(",
    ")",
    "round(",
    "pxl-Test(",
    "augment(",
    "rowSwap(",
    "row+(",
    "*row(",
    "*row+(",
    "max(",
    "min(",
    "R►Pr(",
    "R►Pθ(",
    "P►Rx(",
    "P►Ry(",
    "median(",
    "randM(",
    "mean(",
    "solve(",
    "seq(",
    "fnInt(",
    "nDeriv(",
    "",
    "0",
    "fMax(",
    " ",
    "\"",
    ",",
    "i",
    "!",
    "CubicReg ",
    "QuartReg ",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ".",
    "ᴇ",
    " or ",
    " xor ",
    ":",
    "\n",
    " and ",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "θ",
    "",
    "",
    "",
    "prgm",
    "",
    "",
    "",
    "",
    "Radian",
    "Degree",
    "Normal",
    "Sci",
    "Eng",
    "Float",
    "=",
    "<",
    ">",
    "≤",
    "≥",
    "≠",
    "+",
    "-",
    "Ans",
    "Fix ",
    "Horiz",
    "Full",
    "Func",
    "Param",
    "Polar",
    "Seq",
    "IndpntAuto",
    "IndpntAsk",
    "DependAuto",
    "DependAsk",
    "",
    "□",
    "﹢",
    "·",
    "*",
    "/",
    "Trace",
    "ClrDraw",
    "ZStandard",
    "ZTrig",
    "ZBox",
    "Zoom In",
    "Zoom Out",
    "ZSquare",
    "ZInteger",
    "ZPrevious",
    "ZDecimal",
    "ZoomStat",
    "ZoomRcl",
    "PrintScreen",
    "ZoomSto",
    "Text(",
    " nPr ",
    " nCr ",
    "FnOn ",
    "FnOff ",
    "StorePic ",
    "RecallPic ",
    "StoreGDB ",
    "RecallGDB ",
    "Line(",
    "Vertical ",
    "Pt-On(",
    "Pt-Off(",
    "Pt-Change(",
    "Pxl-On(",
    "Pxl-Off(",
    "Pxl-Change(",
    "Shade(",
    "Circle(",
    "Horizontal ",
    "Tangent(",
    "DrawInv ",
    "DrawF ",
    "",
    "rand",
    "π",
    "getKey",
    "'",
    "?",
    "⁻",
    "int(",
    "abs(",
    "det(",
    "identity(",
    "dim(",
    "sum(",
    "prod(",
    "not(",
    "iPart(",
    "fPart(",
    "",
    "√(",
    "³√(",
    "ln(",
    "e^(",
    "log(",
    "₁₀^(",
    "sin(",
    "sin⁻¹(",
    "cos(",
    "cos⁻¹(",
    "tan(",
    "tan⁻¹(",
    "sinh(",
    "sinh⁻¹(",
    "cosh(",
    "soch⁻¹(",
    "tanh(",
    "tanh⁻¹(",
    "If ",
    "Then",
    "Else",
    "While ",
    "Repeat ",
    "For(",
    "End",
    "Return",
    "Lbl ",
    "Goto ",
    "Pause ",
    "Stop",
    "IS>(",
    "DS<(",
    "Input ",
    "Prompt ",
    "Disp ",
    "DispGraph",
    "Output(",
    "ClrHome",
    "Fill(",
    "SortA(",
    "SortD(",
    "DispTable",
    "Menu(",
    "Send(",
    "Get(",
    "PlotsOn ",
    "PlotsOff ",
    "⌊",
    "Plot1(",
    "Plot2(",
    "Plot3(",
    "",
    "^",
    "ˣ√",
    "1-Var Stats ",
    "2-Var Stats ",
    "LinReg(a+bx) ",
    "ExpReg ",
    "LnReg ",
    "PwrReg ",
    "Med-Med ",
    "QuadReg ",
    "ClrList ",
    "ClrTable",
    "Histogram",
    "xyLine",
    "Scatter",
    "LinReg(ax+b) ",
    "[A]",
    "[B]",
    "[C]",
    "[D]",
    "[E]",
    "[F]",
    "[G]",
    "[H]",
    "[I]",
    "[J]",
    "L₁",
    "L₂",
    "L₃",
    "L₄",
    "L₅",
    "L₆",
    "Y₁",
    "Y₂",
    "Y₃",
    "Y₄",
    "Y₅",
    "Y₆",
    "Y₇",
    "Y₈",
    "Y₉",
    "Y₀",
    "X₁ᴛ",
    "Y₁ᴛ",
    "X₂ᴛ",
    "Y₂ᴛ",
    "X₃ᴛ",
    "Y₃ᴛ",
    "X₄ᴛ",
    "Y₄ᴛ",
    "X₅ᴛ",
    "Y₅ᴛ",
    "X₆ᴛ",
    "Y₆ᴛ",
    "r₁",
    "r₂",
    "r₃",
    "r₄",
    "r₅",
    "r₆",
    "|u",
    "|v",
    "|w",
    "Pic1",
    "Pic2",
    "Pic3",
    "Pic4",
    "Pic5",
    "Pic6",
    "Pic7",
    "Pic8",
    "Pic9",
    "Pic0",
    "GDB1",
    "GDB2",
    "GDB3",
    "GDB4",
    "GDB5",
    "GDB6",
    "GDB7",
    "GDB8",
    "GDB9",
    "GDB0",
    "RegEQ",
    "n",
    "x̄",
    "Σx",
    "Σx²",
    "[Sx]",
    "σx",
    "[minX]",
    "[maxX]",
    "[minY]",
    "[maxY]",
    "ȳ",
    "Σy",
    "Σy²",
    "[Sy]",
    "σy",
    "Σxy",
    "[r]",
    "[Med]",
    "Q₁",
    "Q₃",
    "[|a]",
    "[|b]",
    "[|c]",
    "[|d]",
    "[|e]",
    "x₁",
    "x₂",
    "x₃",
    "y₁",
    "y₂",
    "y₃",
    "𝒏",
    "[p]",
    "[z]",
    "[t]",
    "χ²",
    "[|F]",
    "[df]",
    "p̂",
    "p̂₁",
    "p̂₂",
    "x̄₁",
    "Sx₁",
    "n₁",
    "x̄₂",
    "Sx₂",
    "n₂",
    "Sxp",
    "lower",
    "upper",
    "[s]",
    "r²",
    "R²",
    "[factordf]",
    "[factorSS]",
    "[factorMS]",
    "[errordf]",
    "[errorSS]",
    "[errorMS]",
    "ZXscl",
    "ZYscl",
    "Xscl",
    "Yscl",
    "u(nMin)",
    "v(nMin)",
    "Un-₁",
    "Vn-₁",
    "Zu(nmin)",
    "Zv(nmin)",
    "Xmin",
    "Xmax",
    "Ymin",
    "Ymax",
    "Tmin",
    "Tmax",
    "θMin",
    "θMax",
    "ZXmin",
    "ZXmax",
    "ZYmin",
    "ZYmax",
    "Zθmin",
    "Zθmax",
    "ZTmin",
    "ZTmax",
    "TblStart",
    "PlotStart",
    "ZPlotStart",
    "nMax",
    "ZnMax",
    "nMin",
    "ZnMin",
    "∆Tbl",
    "Tstep",
    "θstep",
    "ZTstep",
    "Zθstep",
    "∆X",
    "∆Y",
    "XFact",
    "YFact",
    "TblInput",
    "𝗡",
    "I%",
    "PV",
    "PMT",
    "FV",
    "|P/Y",
    "|C/Y",
    "w(nMin)",
    "Zw(nMin)",
    "PlotStep",
    "ZPlotStep",
    "Xres",
    "ZXres",
    "TraceStep",
    "Sequential",
    "Simul",
    "PolarGC",
    "RectGC",
    "CoordOn",
    "CoordOff",
    "Connected",
    "Thick",
    "Dot",
    "Dot-Thick",
    "AxesOn ",
    "AxesOff",
    "GridDot ",
    "GridOn",
    "GridOff",
    "LabelOn",
    "LabelOff",
    "Web",
    "Time",
    "uvAxes",
    "vwAxes",
    "uwAxes",
    "Str1",
    "Str2",
    "Str3",
    "Str4",
    "Str5",
    "Str6",
    "Str7",
    "Str8",
    "Str9",
    "Str0",
    "npv(",
    "irr(",
    "bal(",
    "ΣPrn(",
    "ΣInt(",
    "►Nom(",
    "►Eff(",
    "dbd(",
    "lcm(",
    "gcd(",
    "randInt(",
    "randBin(",
    "sub(",
    "stdDev(",
    "variance(",
    "inString(",
    "normalcdf(",
    "invNorm(",
    "tcdf(",
    "χ²cdf(",
    "Fcdf(",
    "binompdf(",
    "binomcdf(",
    "poissonpdf(",
    "poissoncdf(",
    "geometpdf(",
    "geometcdf(",
    "normalpdf(",
    "tpdf(",
    "χ²pdf(",
    "Fpdf(",
    "randNorm(",
    "tvm_Pmt",
    "tvm_I%",
    "tvm_PV",
    "tvm_N",
    "tvm_FV",
    "conj(",
    "real(",
    "imag(",
    "angle(",
    "cumSum(",
    "expr(",
    "length(",
    "ΔList(",
    "ref(",
    "rref(",
    "►Rect",
    "►Polar",
    "e",
    "SinReg ",
    "Logistic ",
    "LinRegTTest ",
    "ShadeNorm(",
    "Shade_t(",
    "Shadeχ²(",
    "ShadeF(",
    "Matr►list(",
    "List►matr(",
    "Z-Test(",
    "T-Test ",
    "2-SampZTest(",
    "1-PropZTest(",
    "2-PropZTest(",
    "χ²-Test(",
    "ZInterval ",
    "2-SampZInt(",
    "1-PropZInt(",
    "2-PropZInt(",
    "GraphStyle(",
    "2-SampTTest ",
    "2-SampFTest ",
    "TInterval ",
    "2-SampTInt ",
    "SetUpEditor ",
    "Pmt_End",
    "Pmt_Bgn",
    "Real",
    "re^θi",
    "a+bi",
    "ExprOn",
    "ExprOff",
    "ClrAllLists",
    "GetCalc(",
    "DelVar ",
    "Equ►String(",
    "String►Equ(",
    "Clear Entries",
    "Select(",
    "ANOVA(",
    "ModBoxplot",
    "NormProbPlot",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "G-T",
    "ZoomFit",
    "DiagnosticOn",
    "DiagnosticOff",
    "Archive ",
    "UnArchive ",
    "Asm(",
    "AsmComp(",
    "AsmPrgm",
    "compiled asm",
    "Á",
    "À",
    "Â",
    "Ä",
    "á",
    "à",
    "â",
    "ä",
    "É",
    "È",
    "Ê",
    "Ë",
    "é",
    "è",
    "ê",
    "ë",
    "",
    "Ì",
    "Î",
    "Ï",
    "í",
    "ì",
    "î",
    "ï",
    "Ó",
    "Ò",
    "Ô",
    "Ö",
    "ó",
    "ò",
    "ô",
    "ö",
    "Ú",
    "Ù",
    "Û",
    "Ü",
    "ú",
    "ù",
    "û",
    "ü",
    "Ç",
    "ç",
    "Ñ",
    "ñ",
    "´",
    "`",
    "¨",
    "¿",
    "¡",
    "α",
    "β",
    "γ",
    "Δ",
    "δ",
    "ε",
    "λ",
    "μ",
    "|π",
    "ρ",
    "Σ",
    "",
    "Φ",
    "Ω",
    "ṗ",
    "χ",
    "𝐅",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "σ",
    "τ",
    "Í",
    "GarbageCollect",
    "~",
    "@",
    "#",
    "$",
    "&",
    "‛",
    ""    "",
    "\\",
    "|",
    "_",
    "'",
    "%",
    "…",
    "∠",
    "ß",
    "ˣ",
    "ᴛ",
    "₀",
    "₁",
    "₂",
    "₃",
    "₄",
    "₅",
    "₆",
    "₇",
    "₈",
    "₉",
    "₁₀",
    "◄",
    "►",
    "↑",
    "↓",
    "×",
    "∫",
    "🡅",
    "🡇",
    "√",
    "setDate(",
    "setTime(",
    "checkTmr(",
    "setDtFmt(",
    "setTmFmt(",
    "timeCnv(",
    "dayOfWk(",
    "getDtStr(",
    "getTmStr(",
    "getDate",
    "getTime",
    "startTmr",
    "getDtFmt",
    "getTmFmt",
    "isClockOn",
    "ClockOff",
    "ClockOn",
    "OpenLib(",
    "ExecLib",
    "invT(",
    "χ²GOF-Test(",
    "LinRegTInt ",
    "Manual-Fit ",
    "ZQuadrant1",
    "ZFrac1⁄2",
    "ZFrac1⁄3",
    "ZFrac1⁄4",
    "ZFrac1⁄5",
    "ZFrac1⁄8",
    "ZFrac1⁄10",
    "⬚",
    "⁄",
    "ᵤ",
    "►n⁄d◄►Un⁄d",
    "►F◄►D",
    "remainder(",
    "Σ(",
    "logBASE(",
    "randIntNoRep(",
    "MATHPRINT",
    "CLASSIC",
    "n⁄d",
    "Un⁄d",
    "AUTO",
    "DEC",
    "FRAC",
    "FRAC-APPROX",
    "BLUE",
    "RED",
    "BLACK",
    "MAGENTA",
    "GREEN",
    "ORANGE",
    "BROWN",
    "NAVY",
    "LTBLUE",
    "YELLOW",
    "WHITE",
    "LTGRAY",
    "MEDGRAY",
    "GRAY",
    "DARKGRAY",
    "Image1",
    "Image2",
    "Image3",
    "Image4",
    "Image5",
    "Image6",
    "Image7",
    "Image8",
    "Image9",
    "Image0",
    "GridLine ",
    "BackgroundOn ",
    "BackgroundOff",
    "GraphColor(",
    "QuickPlot&Fit-EQ",
    "TextColor(",
    "Asm84CPrgm",
    "",
    "DetectAsymOn",
    "DetectAsymOff",
    "BorderColor ",
    "invBinom(",
    "Wait ",
    "toString(",
    "eval("
};

int grabString(uint8_t **outputPtr, bool stopAtStoreAndString) {
    char tempString[16];
    uint24_t output, a;

    while (1) {
        int token = _getc();
        uint8_t tok = token;

        if (tok == tEnter || token == EOF || (stopAtStoreAndString && (tok == tStore || tok == tString))) {
            return token;
        }

        if (tok == 0x5C) {
            output = 255 + _getc();
        } else if (tok == 0x5D) {
            output = 265 + _getc();
        } else if (tok == 0x5E) {
            tok = _getc();
            output = 271 + tok - 16 - 6 * (tok >= 0x20) - 20 * (tok >= 0x40) - 58 * (tok >= 0x80);
        } else if (tok == 0x60) {
            output = 302 + _getc();
        } else if (tok == 0x61) {
            output = 312 + _getc();
        } else if (tok == 0x62) {
            output = 322 + _getc() - 1;
        } else if (tok == 0x63) {
            output = 382 + _getc();
        } else if (tok == 0x7E) {
            output = 439 + _getc();
        } else if (tok == 0xAA) {
            output = 461 + _getc();
        } else if (tok == 0xBB) {
            output = 471 + _getc();
        } else if (tok == 0xEF) {
            tok = _getc();
            output = 715 + tok - 15 * (tok >= 0x2E) - 3 * (tok >= 0x40) - 8 * (tok >= 0x60) - 40 * (tok >= 0x70);
        } else if (tok) {
            output = token - 1;
        } else {
            displayError(W_WRONG_CHAR);
            continue;
        }

        strcpy(tempString, tokenStrings[output]);
        memcpy(*outputPtr, tempString, strlen(tempString));
        *outputPtr += strlen(tempString);

        for (a = 0; a < strlen(tempString); a++) {
            uint8_t char2 = tempString[a];

            // No weird characters
            if (char2 < 32 || char2 > 127) {
                displayError(W_WRONG_CHAR);
                char2 = 0;
            }
        }
    }
}

int getNextToken(void) {
    if ((uint32_t)_tell(ice.inPrgm) < ice.programLength - 2) {
        return fgetc(ice.inPrgm);
    }
    
    return EOF;
}

#endif
