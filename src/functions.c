#include "defines.h"
#include "ast.h"
#include "functions.h"

#include "errors.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "routines.h"
#include "prescan.h"
#include "ast.h"

#ifndef CALCULATOR
extern const uint8_t SqrtData[];
extern const uint8_t MeanData[];
extern const uint8_t RandData[];
extern const uint8_t TimerData[];
extern const uint8_t MallocData[];
extern const uint8_t SincosData[];
extern const uint8_t KeypadData[];
extern const uint8_t RandintData[];
extern const uint8_t LoadspriteData[];
extern const uint8_t LoadtilemapData[];
#endif

/* First byte:  bit 7  : returns something in A
                bit 6  : unimplemented
                bit 5  : returns something in HL (16 bits)
                bit 4  : deprecated
                bit 2-0: amount of arguments needed
   Second byte: bit 7  : first argument is small
                bit 6  : second argument is small
                bit 5  : third argument is small
                ...
*/

static const c_function_t GraphxArgs[] = {
    {RET_NONE | 0, ARG_NORM},    // Begin
    {RET_NONE | 0, ARG_NORM},    // End
    {RET_A    | 1, SMALL_1},     // SetColor
    {RET_NONE | 0, ARG_NORM},    // SetDefaultPalette
    {RET_NONE | 3, ARG_NORM},    // SetPalette
    {RET_NONE | 1, SMALL_1},     // FillScreen
    {RET_NONE | 2, SMALL_2},     // SetPixel
    {RET_A    | 2, SMALL_2},     // GetPixel
    {RET_A    | 0, ARG_NORM},    // GetDraw
    {RET_NONE | 1, SMALL_1},     // SetDraw
    {RET_NONE | 0, ARG_NORM},    // SwapDraw
    {RET_NONE | 1, SMALL_1},     // Blit
    {RET_NONE | 3, SMALL_123},   // BlitLines
    {RET_NONE | 5, SMALL_13},    // BlitArea
    {RET_NONE | 1, SMALL_1},     // PrintChar
    {RET_NONE | 2, SMALL_2},     // PrintInt
    {RET_NONE | 2, SMALL_2},     // PrintUInt
    {RET_NONE | 1, ARG_NORM},    // PrintString
    {RET_NONE | 3, ARG_NORM},    // PrintStringXY
    {RET_NONE | 2, ARG_NORM},    // SetTextXY
    {RET_A    | 1, SMALL_1},     // SetTextBGColor
    {RET_A    | 1, SMALL_1},     // SetTextFGColor
    {RET_A    | 1, SMALL_1},     // SetTextTransparentColor
    {RET_NONE | 1, ARG_NORM},    // SetCustomFontData
    {RET_NONE | 1, ARG_NORM},    // SetCustomFontSpacing
    {RET_NONE | 1, SMALL_1},     // SetMonoSpaceFont
    {RET_HL   | 1, ARG_NORM},    // GetStringWidth
    {RET_HL   | 1, SMALL_1},     // GetCharWidth
    {RET_HL   | 0, ARG_NORM},    // GetTextX
    {RET_HL   | 0, ARG_NORM},    // GetTextY
    {RET_NONE | 4, ARG_NORM},    // Line
    {RET_NONE | 3, ARG_NORM},    // HorizLine
    {RET_NONE | 3, ARG_NORM},    // VertLine
    {RET_NONE | 3, ARG_NORM},    // Circle
    {RET_NONE | 3, ARG_NORM},    // FillCircle
    {RET_NONE | 4, ARG_NORM},    // Rectangle
    {RET_NONE | 4, ARG_NORM},    // FillRectangle
    {RET_NONE | 4, SMALL_24},    // Line_NoClip
    {RET_NONE | 3, SMALL_2},     // HorizLine_NoClip
    {RET_NONE | 3, SMALL_2},     // VertLine_NoClip
    {RET_NONE | 3, SMALL_2},     // FillCircle_NoClip
    {RET_NONE | 4, SMALL_24},    // Rectangle_NoClip
    {RET_NONE | 4, SMALL_24},    // FillRectangle_NoClip
    {RET_NONE | 4, ARG_NORM},    // SetClipRegion
    {RET_A    | 1, ARG_NORM},    // GetClipRegion
    {RET_NONE | 1, SMALL_1},     // ShiftDown
    {RET_NONE | 1, SMALL_1},     // ShiftUp
    {RET_NONE | 1, ARG_NORM},    // ShiftLeft
    {RET_NONE | 1, ARG_NORM},    // ShiftRight
    {RET_NONE | 3, ARG_NORM},    // Tilemap
    {RET_NONE | 3, ARG_NORM},    // Tilemap_NoClip
    {RET_NONE | 3, ARG_NORM},    // TransparentTilemap
    {RET_NONE | 3, ARG_NORM},    // TransparentTilemap_NoClip
    {RET_HL   | 3, ARG_NORM},    // TilePtr
    {RET_HL   | 3, SMALL_23},    // TilePtrMapped
    {UN       | 0, ARG_NORM},    // LZDecompress
    {UN       | 0, ARG_NORM},    // AllocSprite
    {RET_NONE | 3, ARG_NORM},    // Sprite
    {RET_NONE | 3, ARG_NORM},    // TransparentSprite
    {RET_NONE | 3, SMALL_3},     // Sprite_NoClip
    {RET_NONE | 3, SMALL_3},     // TransparentSprite_NoClip
    {RET_HL   | 3, ARG_NORM},    // GetSprite
    {RET_NONE | 5, SMALL_345},   // ScaledSprite_NoClip
    {RET_NONE | 5, SMALL_345},   // ScaledTransparentSprite_NoClip
    {RET_HL   | 2, ARG_NORM},    // FlipSpriteY
    {RET_HL   | 2, ARG_NORM},    // FlipSpriteX
    {RET_HL   | 2, ARG_NORM},    // RotateSpriteC
    {RET_HL   | 2, ARG_NORM},    // RotateSpriteCC
    {RET_HL   | 2, ARG_NORM},    // RotateSpriteHalf
    {RET_NONE | 2, ARG_NORM},    // Polygon
    {RET_NONE | 2, ARG_NORM},    // Polygon_NoClip
    {RET_NONE | 6, ARG_NORM},    // FillTriangle
    {RET_NONE | 6, ARG_NORM},    // FillTriangle_NoClip
    {UN       | 0, ARG_NORM},    // LZDecompressSprite
    {RET_NONE | 2, SMALL_12},    // SetTextScale
    {RET_A    | 1, SMALL_1},     // SetTransparentColor
    {RET_NONE | 0, ARG_NORM},    // ZeroScreen
    {RET_NONE | 1, SMALL_1},     // SetTextConfig
    {RET_HL   | 1, SMALL_1},     // GetSpriteChar
    {RET_HLs  | 2, SMALL_2},     // Lighten
    {RET_HLs  | 2, SMALL_2},     // Darken
    {RET_A    | 1, SMALL_1},     // SetFontHeight
    {RET_HL   | 2, ARG_NORM},    // ScaleSprite
    {RET_NONE | 3, SMALL_23},    // FloodFill
    {RET_NONE | 3, ARG_NORM},    // RLETSprite
    {RET_NONE | 3, SMALL_3},     // RLETSprite_NoClip
    {RET_HL   | 2, ARG_NORM},    // ConvertFromRLETSprite
    {RET_HL   | 2, ARG_NORM},    // ConvertToRLETSprite
    {UN       | 2, ARG_NORM},    // ConvertToNewRLETSprite
    {RET_HL   | 4, SMALL_34},    // RotateScaleSprite
    {RET_HL   | 5, SMALL_345},   // RotatedScaledTransparentSprite_NoClip
    {RET_HL   | 5, SMALL_345},   // RotatedScaledSprite_NoClip
    {RET_HL   | 2, SMALL_1},     // SetCharData
    {RET_NONE | 0, ARG_NORM},    // Wait
};

static const c_function_t FileiocArgs[] = {
    {RET_NONE | 0, ARG_NORM},    // CloseAll
    {RET_A    | 2, ARG_NORM},    // Open
    {RET_A    | 3, SMALL_3},     // OpenVar
    {RET_NONE | 1, SMALL_1},     // Close
    {RET_HL   | 4, SMALL_4},     // Write
    {RET_HL   | 4, SMALL_4},     // Read
    {RET_HL   | 1, SMALL_1},     // GetChar
    {RET_HL   | 2, SMALL_12},    // PutChar
    {RET_HL   | 1, ARG_NORM},    // Delete
    {RET_HL   | 2, SMALL_2},     // DeleteVar
    {RET_HL   | 3, SMALL_23},    // Seek
    {RET_HL   | 2, SMALL_2},     // Resize
    {RET_HL   | 1, SMALL_1},     // IsArchived
    {RET_NONE | 2, SMALL_12},    // SetArchiveStatus
    {RET_HL   | 1, SMALL_1},     // Tell
    {RET_HL   | 1, SMALL_1},     // Rewind
    {RET_HL   | 1, SMALL_1},     // GetSize
    {RET_HL   | 3, ARG_NORM},    // GetTokenString
    {RET_HL   | 1, SMALL_1},     // GetDataPtr
    {RET_HL   | 2, ARG_NORM},    // Detect
    {RET_HL   | 3, SMALL_3},     // DetectVar
    {UN       | 3, SMALL_1},     // SetVar
    {UN       | 4, SMALL_13},    // StoVar
    {UN       | 3, SMALL_1},     // RclVar
    {UN       | 2, ARG_NORM},    // AllocString
    {UN       | 2, ARG_NORM},    // AllocList
    {UN       | 3, SMALL_12},    // AllocMatrix
    {UN       | 2, ARG_NORM},    // AllocCplxList
    {UN       | 2, ARG_NORM},    // AllocEqu
    {RET_HL   | 3, ARG_NORM},    // DetectAny
    {RET_HL   | 1, SMALL_1},     // GetVATPtr
    {RET_NONE | 2, SMALL_2},     // GetName
    {RET_A    | 2, ARG_NORM},    // Rename
    {RET_A    | 3, SMALL_3},     // RenameVar
};

const function_t implementedFunctions[AMOUNT_OF_FUNCTIONS] = {
// function / second byte / amount of args / allow args as numbers / args backwards pushed
    {tNot,      0,              1,   1, 0},
    {tMin,      0,              2,   1, 0},
    {tMax,      0,              2,   1, 0},
    {tMean,     0,              2,   1, 0},
    {tSqrt,     0,              1,   1, 0},
    {tDet,      0,              255, 0, 1},
    {tSum,      0,              255, 0, 1},
    {tSin,      0,              1,   1, 0},
    {tCos,      0,              1,   1, 0},
    {tGetKey,   0,              255, 0, 0},
    {tRand,     0,              0,   0, 0},
    {tAns,      0,              0,   0, 0},
    {tLog,      0,              1,   1, 0},
    {tLParen,   0,              1,   0, 0},
    {tLBrace,   0,              1,   0, 0},
    {tExtTok,   tRemainder,     2,   1, 0},
    {tExtTok,   tCheckTmr,      1,   0, 0},
    {tExtTok,   tStartTmr,      0,   0, 0},
    {tExtTok,   tLEFT,          2,   1, 0},
    {tExtTok,   tRIGHT,         2,   1, 0},
    {tExtTok,   tToString,      1,   0, 0},
    {t2ByteTok, tSubStrng,      3,   0, 0},
    {t2ByteTok, tLength,        1,   0, 0},
    {t2ByteTok, tFinDBD,        1,   0, 0},
    {t2ByteTok, tRandInt,       2,   0, 0},
    {t2ByteTok, tInStrng,       2,   0, 1},
    {tVarOut,   tDefineSprite,  255, 0, 0},
    {tVarOut,   tData,          255, 0, 0},
    {tVarOut,   tCopy,          255, 0, 0},
    {tVarOut,   tAlloc,         1,   0, 0},
    {tVarOut,   tDefineTilemap, 255, 0, 0},
    {tVarOut,   tCopyData,      255, 0, 0},
    {tVarOut,   tLoadData,      3,   0, 0},
    {tVarOut,   tSetBrightness, 1,   0, 0},
    {tVarOut,   tCompare,       2,   0, 1} 
};

uint24_t executeFunction(NODE *top) {
    uint8_t function = top->data.operand.func.function;
    uint8_t function2 = top->data.operand.func.function2;
    uint24_t operand1 = top->child->data.operand.num;
    uint24_t operand2 = top->child->sibling->data.operand.num;
    
    switch (function) {
        case tNot:
            return !operand1;
        case tMin:
            return operand1 < operand2 ? operand1 : operand2;
        case tMax:
            return operand1 > operand2 ? operand1 : operand2;
        case tMean:
            return ((long)operand1 + (long)operand2) / 2;
        case tSqrt:
            return sqrt(operand1);
        case tLog:
            return log10(operand1);
        case tSin:
            return 255 * sin((double)operand1 * (2 * M_PI / 256));
        case tCos:
            return 255 * cos((double)operand1 * (2 * M_PI / 256));
        default:
            if (function2 == tRemainder) {
                return operand1 % operand2;
            } else if (function2 == tLEFT) {
                return operand1 << operand2;
            } else {
                return operand1 >> operand2;
            }
    }
}

uint8_t parseFunction(NODE *top) {
    NODE *child = top->child;
    NODE *sibling = child->sibling;
    uint24_t childOperand, siblingOperand;
    uint8_t childType, siblingType;
    uint8_t function, function2, amountOfArgs, res = VALID;
    
    childOperand = child->data.operand.num;
    siblingOperand = sibling->data.operand.num;
    childType = child->data.type;
    siblingType = sibling->data.type;
    
    function = top->data.operand.func.function;
    function2 = top->data.operand.func.function2;
    amountOfArgs = top->data.operand.func.amountOfArgs;
    
    // (
    if (function == tLParen) {
        expr.outputReturnRegister = expr.outputRegister;
    }
    
    // not(
    else if (function == tNot) {
        res = parseNode(child);
        
        if (expr.outputRegister == REGISTER_A) {
            OutputWrite2Bytes(OP_ADD_A, 255);
            OutputWrite2Bytes(OP_SBC_A_A, OP_INC_A);
            ResetA();
            expr.outputReturnRegister = REGISTER_A;
            if (expr.AnsSetZeroFlag || expr.AnsSetCarryFlag || expr.AnsSetZeroFlagReversed || expr.AnsSetCarryFlagReversed) {
                bool temp = expr.AnsSetZeroFlag;

                expr.ZeroCarryFlagRemoveAmountOfBytes += 4;
                expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed;
                expr.AnsSetZeroFlagReversed = temp;
                temp = expr.AnsSetCarryFlag;
                expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed;
                expr.AnsSetCarryFlagReversed = temp;
            } else {
                expr.ZeroCarryFlagRemoveAmountOfBytes = 2;
                expr.AnsSetCarryFlag = true;
            }
        } else {
            uint8_t *tempProgPtr = ice.programPtr;
            
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(-1, TYPE_NUMBER);
            } else {
                OutputWrite3Bytes(OP_SCF, 0xED, 0x62);
            }
            ADD_HL_DE();
            SBC_HL_HL_INC_HL();
            if (expr.ZeroCarryFlagRemoveAmountOfBytes) {
                bool temp = expr.AnsSetZeroFlag;

                expr.ZeroCarryFlagRemoveAmountOfBytes += ice.programPtr - tempProgPtr;
                expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed;
                expr.AnsSetZeroFlagReversed = temp;
                temp = expr.AnsSetCarryFlag;
                expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed;
                expr.AnsSetCarryFlagReversed = temp;
            } else {
                ClearAnsFlags();
                expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
                expr.AnsSetCarryFlag = true;
            }
        }
    } else {
        ClearAnsFlags();
        
        // sin(, cos(
        if (function == tSin || function == tCos) {
            res = parseNode(child);
            AnsToHL();
            
            if (!ice.usedAlreadySinCos) {
                ice.programDataPtr -= SIZEOF_SINCOS_DATA;
                ice.SinCosAddr = (uintptr_t)ice.programDataPtr;
                memcpy(ice.programDataPtr, SincosData, SIZEOF_SINCOS_DATA);

                // 16 = distance from start of routine to "ld de, SinTable"
                ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.programDataPtr + 16);

                // This is the "ld de, SinTable", 18 is the distance from "ld de, SinTable" to "SinTable"
                *(uint24_t*)(ice.programDataPtr + 16) = (uint24_t)ice.programDataPtr + 18 + 16;
                ice.usedAlreadySinCos = true;
            }

            ProgramPtrToOffsetStack();
            CALL(ice.SinCosAddr + (function == tSin ? 4 : 0));
            ResetAllRegs();

            expr.outputReturnRegister = REGISTER_DE;
        }
        
        // rand
        else if (function == tRand) {
            ProgramPtrToOffsetStack();
            CALL(ice.randAddr + SIZEOF_SRAND_DATA);

            ice.modifiedIY = true;
            ResetAllRegs();
        }
        
        // Ans - only works with numbers < 1000!
        else if (function == tAns) {
            CALL(_RclAns);
            CALL(_ConvOP1);
            ResetAllRegs();
            expr.outputReturnRegister = REGISTER_DE;
        }
        
        // sqrt(
        else if (function == tSqrt) {
            CallRoutine(&ice.usedAlreadySqrt, &ice.SqrtAddr, (uint8_t*)SqrtData, SIZEOF_SQRT_DATA);
            ResetAllRegs();

            expr.outputReturnRegister = REGISTER_DE;
            ice.modifiedIY = true;
        }
        
        // min(, max(
        else if (function == tMin || function == tMax) {
            if ((res = parseFunction2Args(child, sibling, REGISTER_DE, false)) != VALID) {
                return res;
            }

            OR_A_SBC_HL_DE();
            ADD_HL_DE();
            if (function == tMin) {
                OutputWrite2Bytes(OP_JR_C, 1);
            } else {
                OutputWrite2Bytes(OP_JR_NC, 1);
            }
            EX_DE_HL();
            ResetHL();                 // DE is already reset because of "add hl, de \ ex de, hl"
        }
        
        // mean(
        else if (function == tMean) {
            if ((res = parseFunction2Args(child, sibling, REGISTER_DE, false)) != VALID) {
                return res;
            }

            CallRoutine(&ice.usedAlreadyMean, &ice.MeanAddr, (uint8_t*)MeanData, SIZEOF_MEAN_DATA);
            ResetHL();
            ResetBC();
            ResetA();
        }
        
        // getKey / getKey(X)
        else if (function == tGetKey) {
            if (amountOfArgs > 1) {
                return E_ARGUMENTS;
            }
            if (amountOfArgs) {
                if (childType == TYPE_STRING) {
                    return E_SYNTAX;
                } else if (childType == TYPE_NUMBER) {
                    uint8_t key = childOperand;
                    uint8_t keyBit = 1;
                    
                    /* ((key-1)/8 & 7) * 2 =
                       (key-1)/4 & (7*2) =
                       (key-1) >> 2 & 14  */
                    LD_B(0x1E - (((key - 1) >> 2) & 14));

                    // Get the right bit for the keypress
                    key = (key - 1) & 7;
                    while (key--) {
                        keyBit = keyBit << 1;
                    }

                    LD_C(keyBit);
                } else if (childType == TYPE_VARIABLE) {
                    LD_A_IND_IX_OFF(childOperand);
                } else {
                    parseNode(child->child);
                }

                if (childType != TYPE_NUMBER) {
                    if (expr.outputRegister == REGISTER_A) {
                        OutputWrite2Bytes(OP_DEC_A, OP_LD_D_A);
                        loadGetKeyFastData1();
                        LD_A_D();
                    } else {
                        if (expr.outputRegister == REGISTER_HL) {
                            LD_A_L();
                        } else {
                            LD_A_E();
                        }
                        loadGetKeyFastData1();
                        if (expr.outputRegister == REGISTER_HL) {
                            LD_A_L();
                        } else {
                            LD_A_E();
                        }
                    }
                    loadGetKeyFastData2();
                }

                CallRoutine(&ice.usedAlreadyGetKeyFast, &ice.getKeyFastAddr, (uint8_t*)KeypadData, SIZEOF_KEYPAD_DATA);
                ResetHL();
                ResetA();
            } else {
                CALL(_os_GetCSC);
                ResetHL();
                ResetA();
                ice.modifiedIY = false;
            }
        }
        
        // {
        else if (function == tLBrace) {
            uint8_t mask = function2;
            
            if (childType <= TYPE_NUMBER) {
                if (childType == TYPE_STRING && !comparePtrToTempStrings(childOperand)) {
                    ProgramPtrToOffsetStack();
                }
                if (mask == TYPE_MASK_U8) {
                    LD_A_ADDR(childOperand);
                } else {
                    LD_HL_ADDR(childOperand);
                }
            } else if (childType == TYPE_VARIABLE) {
                LD_HL_IND_IX_OFF(childOperand);
                if (mask == TYPE_MASK_U8) {
                    LD_A_HL();
                } else {
                    LD_HL_HL();
                }
            } else {
                res = parseNode(child);
                MaybeAToHL();
                if (mask == TYPE_MASK_U8) {
                    if (expr.outputRegister == REGISTER_DE) {
                        LD_A_DE();
                    } else {
                        LD_A_HL();
                    }
                } else {
                    AnsToHL();
                    LD_HL_HL();
                }
            }
            
            if (mask == TYPE_MASK_U16) {
                EX_S_DE_HL();
                expr.outputReturnRegister = REGISTER_DE;
            }
            
            if (mask == TYPE_MASK_U8) {
                expr.outputReturnRegister = REGISTER_A;
            }
        }
        
        else if (function == t2ByteTok && function2 != tInStrng) {
            // randInt(
            if (function2 == tRandInt) {
                if (childType == TYPE_STRING || siblingType == TYPE_STRING) {
                    return E_SYNTAX;
                }
                
                if ((res = parseFunction2Args(child, sibling, REGISTER_DE, true)) != VALID) {
                    return res;
                }
                
                CallRoutine(&ice.usedAlreadyRandInt, &ice.RandIntAddr, (uint8_t*)RandintData, SIZEOF_RANDINT_DATA);
                ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.RandIntAddr + 8);
                w24(ice.RandIntAddr + 8, ice.randAddr + SIZEOF_SRAND_DATA);
                ResetHL();
                ResetBC();
                ResetA();
            }
            
            // sub(
            else if (function2 == tSubStrng) {
                // Parse last 2 argument
                if ((res = parseFunction2Args(sibling, sibling->sibling, REGISTER_BC, true)) != VALID) {
                    return res;
                }

                // Get the string into DE
                LD_DE_IMM(childOperand, childType);

                // Add the offset to the string, and copy!
                ADD_HL_DE();
                if (childOperand == prescan.tempStrings[TempString1]) {
                    LD_DE_IMM(prescan.tempStrings[TempString2], TYPE_STRING);
                } else {
                    LD_DE_IMM(prescan.tempStrings[TempString1], TYPE_STRING);
                }
                OutputWrite3Bytes(OP_PUSH_DE, 0xED, 0xB0);
                OutputWrite3Bytes(OP_EX_DE_HL, OP_LD_HL_C, OP_POP_HL);
            }
            
            // length(
            else if (function2 == tLength) {
                res = parseNode(child);
                PushHLDE();
                CALL(__strlen);
                POP_BC();
                if (expr.outputRegister == REGISTER_HL) {
                    reg.BCIsNumber = reg.HLIsNumber;
                    reg.BCIsVariable = reg.HLIsVariable;
                    reg.BCValue = reg.HLValue;
                    reg.BCVariable = reg.HLVariable;
                } else {
                    reg.BCIsNumber = reg.DEIsNumber;
                    reg.BCIsVariable = reg.DEIsVariable;
                    reg.BCValue = reg.DEValue;
                    reg.BCVariable = reg.DEVariable;
                }
                ResetHL();
            }
            
            // dbd( - debug things
            else if (function2 == tFinDBD) {
#ifdef CALCULATOR
                if (!outputPrevOperand) {
                    // Add ice.currentLine to the stack of startup breakpoints
                    ice.breakPointLines[ice.currentBreakPointLine++] = ice.currentLine - 1;
                    OutputWriteByte(0);
                } else {
                    return E_SYNTAX;
                }
#else
                fprintf(stdout, "Debugging not allowed - use the calculator version!");
#endif
            }
        }
        
        else if ((function == tExtTok || function == tLog) && function2 != tCompare) {
            // LEFT(
            if (function2 == tLEFT) {
                const uint8_t mem[] = {OP_OR_A, OP_JR_Z, 4, OP_ADD_HL_HL, OP_DEC_A, OP_JR_NZ, -4, 0};
                
                OutputWriteMem(mem);
            }
            
            // RIGHT(
            else if (function2 == tRIGHT) {
                if (childType == TYPE_STRING || siblingType == TYPE_STRING) {
                    return E_SYNTAX;
                }
                
                res = parseFunction2Args(child, sibling, REGISTER_A, true);
                
                OR_A_A();
                
                ProgramPtrToOffsetStack();
                if (!ice.usedAlreadyMean) {
                    ice.programDataPtr -= SIZEOF_MEAN_DATA;
                    ice.MeanAddr = (uintptr_t)ice.programDataPtr;
                    memcpy(ice.programDataPtr, (uint8_t*)MeanData, SIZEOF_MEAN_DATA);
                    ice.usedAlreadyMean = true;
                }
                
                CALL_NZ(ice.MeanAddr + 3);
                ResetHL();
                ResetBC();
                ResetA();
            }
            
            // remainder(
            else if (function2 == tRemainder) {
                if (childType == TYPE_STRING || siblingType == TYPE_STRING) {
                    return E_SYNTAX;
                }
                
                if (siblingType == TYPE_NUMBER && siblingOperand <= 256 && !((uint8_t)siblingOperand & (uint8_t)(siblingOperand - 1))) {
                    if (childType == TYPE_VARIABLE) {
                        LD_A_IND_IX_OFF(childOperand);
                    } else {
                        res = parseNode(child);
                        AnsToA();
                    }
                    if (siblingOperand == 256) {
                        OR_A_A();
                    } else {
                        AND_A(siblingOperand - 1);
                    }
                    SBC_HL_HL();
                    LD_L_A();
                    reg.HLIsNumber = reg.AIsNumber;
                    reg.HLIsVariable = false;
                    reg.HLValue = reg.AValue;
                    reg.HLVariable = reg.AVariable;
                } else {
                    if ((res = parseFunction2Args(child, sibling, REGISTER_BC, true)) != VALID) {
                        return res;
                    }
                    CALL(__idvrmu);
                    ResetHL();
                    ResetDE();
                    reg.AIsNumber = true;
                    reg.AIsVariable = false;
                    reg.AValue = 0;
                }
            }
            
            // startTmr
            else if (function2 == tStartTmr) {
                CallRoutine(&ice.usedAlreadyTimer, &ice.TimerAddr, (uint8_t*)TimerData, SIZEOF_TIMER_DATA);
            }
            
            // checkTmr(
            else if (function2 == tCheckTmr) {
                res = parseNode(child);
                AnsToDE();

                LD_HL_IND(0xF20000);
                OR_A_SBC_HL_DE();
            }
            
            // toString(
            else if (function2 == tToString || function == tLog) {
                res = parseNode(child);
                AnsToHL();
                
                CallRoutine(&ice.usedAlreadyToString, &ice.ToStringAddr, (uint8_t*)TostringData, SIZEOF_TOSTRING_DATA);
                ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.ToStringAddr + 1);
                w24(ice.ToStringAddr + 1, ice.ToStringAddr + SIZEOF_TOSTRING_DATA);
                if (function == tLog) {
                    OutputWrite2Bytes(OP_PUSH_BC, OP_POP_HL);
                } else {
                    LD_DE_IMM(prescan.tempStrings[TempString1], TYPE_NUMBER);
                    OutputWrite3Bytes(OP_PUSH_DE, 0xED, 0xB0);
                    OutputWrite3Bytes(OP_EX_DE_HL, OP_LD_HL_C, OP_POP_HL);
                }
                ResetAllRegs();
            }
        }
        
        else if (function == tVarOut && function2 != tCompare) {
            // Alloc(
            if (function2 == tAlloc) {
                res = parseNode(child);
                AnsToHL();

                InsertMallocRoutine();
            }
            
            // DefineSprite(
            else if (function2 == tDefineSprite) {
                uint8_t width = childOperand;
                uint8_t height = siblingOperand;
                
                /*****************************************************
                * Inputs:
                *  arg1: sprite width
                *  arg2: sprite height
                *  (arg3: sprite data)
                *
                * Returns: PTR to sprite
                *****************************************************/

                if (amountOfArgs == 2) {
                    if (childType != TYPE_NUMBER || siblingType != TYPE_NUMBER) {
                        return E_SYNTAX;
                    }

                    LD_HL_IMM(width * height + 2, TYPE_NUMBER);
                    InsertMallocRoutine();
                    OutputWrite2Bytes(OP_JR_NC, 6);
                    LD_HL_VAL(width);
                    INC_HL();
                    LD_HL_VAL(height);
                    DEC_HL();
                } else if (amountOfArgs == 3) {
                    uint8_t *a;

                    if(childType != TYPE_NUMBER || siblingType != TYPE_NUMBER || sibling->sibling->data.type != TYPE_STRING) {
                        return E_SYNTAX;
                    }

                    ice.programDataPtr -= 2;
                    LD_HL_IMM((uint24_t)ice.programDataPtr, TYPE_STRING);
                    ResetHL();

                    *ice.programDataPtr = width;
                    *(ice.programDataPtr + 1) = height;
                } else {
                    return E_ARGUMENTS;
                }
            }
            
            // Data(
            else if (function2 == tData) {
                /***********************************
                * Inputs:
                *  arg1: size in bytes of each entry
                *  arg2-argX: entries, constants
                *
                * Returns: PTR to data
                ***********************************/

                res = InsertDataElements(child);
            }
            
            // Copy(
            else if (function2 == tCopy) {
                NODE *sibling2 = sibling->sibling;
                uint8_t sibling2Type;
                uint24_t sibling2Operand;
                
                /*****************************************************
                * Inputs:
                *  arg1: PTR to destination
                *  arg2: PTR to source
                *  arg3: size in bytes
                *****************************************************/
                
                if (amountOfArgs == 4) {
                    child = sibling;
                    sibling = sibling2;
                    sibling2 = sibling->sibling;
                    amountOfArgs--;
                }
                
                if (amountOfArgs != 3) {
                    return E_ARGUMENTS;
                }
                
                sibling2Type = sibling2->data.type;
                sibling2Operand = sibling2->data.operand.num;
                
                if (sibling2Type > TYPE_VARIABLE) {
                    if ((res = parseNode(sibling2)) != VALID) {
                        return res;
                    }
                    if (childType <= TYPE_VARIABLE && siblingType <= TYPE_VARIABLE) {
                        AnsToBC();
                    } else {
                        PushHLDE();
                    }
                }
                
                if (childType > TYPE_VARIABLE) {
                    if ((res = parseNode(child)) != VALID) {
                        return res;
                    }
                    if (siblingType <= TYPE_VARIABLE) {
                        AnsToDE();
                    } else {
                        PushHLDE();
                    }
                }
                
                if (siblingType > TYPE_VARIABLE) {
                    if ((res = parseNode(sibling)) != VALID) {
                        return res;
                    }
                    AnsToHL();
                    if (childType > TYPE_VARIABLE) {
                        POP_DE();
                    }
                }
                
                if (sibling2Type > TYPE_VARIABLE) {
                    POP_BC();
                }

                if (childType <= TYPE_STRING) {
                    LD_DE_IMM(childOperand, childType);
                } else if (childType == TYPE_VARIABLE) {
                    LD_DE_IND_IX_OFF(childOperand);
                }
                if (siblingType <= TYPE_STRING) {
                    LD_HL_IMM(siblingOperand, siblingType);
                } else if (siblingType == TYPE_VARIABLE) {
                    LD_HL_IND_IX_OFF(siblingOperand);
                }
                if (sibling2Type <= TYPE_STRING) {
                    LD_BC_IMM(sibling2Operand, sibling2Type);
                } else if (sibling2Type == TYPE_VARIABLE) {
                    LD_BC_IND_IX_OFF(sibling2Operand);
                }
                
                if (amountOfArgs == 4) {
                    LDDR();
                } else {
                    LDIR();
                }
            }
            
            // DefineTilemap(
            else if (function2 == tDefineSprite) {
                uint8_t loop;
                NODE *tmp = child;
                
                /****************************************************************
                * C arguments:
                *  - uint8_t *mapData (pointer to data)
                *  - uint8_t **tilesData (pointer to table with pointers to data)
                *  - uint8_t TILE_HEIGHT
                *  - uint8_t TILE_WIDTH
                *  - uint8_t DRAW_HEIGHT
                *  - uint8_t DRAW_WIDTH
                *  - uint8_t TYPE_WIDTH
                *  - uint8_t TYPE_HEIGHT
                *  - uint8_t HEIGHT
                *  - uint8_t WIDTH
                *  - uint8_t Y_LOC
                *  - uint24_t X_LOC
                *****************************************************************
                * ICE arguments:
                *  - uint8_t TILE_HEIGHT
                *  - uint8_t TILE_WIDTH
                *  - uint8_t DRAW_HEIGHT
                *  - uint8_t DRAW_WIDTH
                *  - uint8_t TYPE_WIDTH
                *  - uint8_t TYPE_HEIGHT
                *  - uint8_t HEIGHT
                *  - uint8_t WIDTH
                *  - uint8_t Y_LOC
                *  - uint24_t X_LOC
                *  - uint8_t **tilesData pointer to data, we have to
                *       create our own table, by getting the size of
                *       each sprite, and thus finding all the sprites
                *  (- uint8_t *mapData (pointer to data))
                *
                * Returns: PTR to tilemap struct
                ****************************************************************/

                if (amountOfArgs < 11 || amountOfArgs > 12) {
                    return E_ARGUMENTS;
                }
                
                child = reverseNode(child);
                
                // Fetch map data
                ice.programDataPtr -= 3;
                if (amountOfArgs == 12) {
                    LD_HL_IMM(child->data.operand.num, TYPE_STRING);
                    ProgramPtrToOffsetStack();
                    LD_ADDR_HL((uint24_t)ice.programDataPtr);
                    child = child->sibling;
                }
                
                // Fetch tiles data
                ice.programDataPtr -= 3;
                if (child->data.type != TYPE_VARIABLE) {
                    res = E_SYNTAX;
                }
                LD_HL_IND_IX_OFF(child->data.operand.num);
                ProgramPtrToOffsetStack();
                LD_ADDR_HL((uint24_t)ice.programDataPtr);
                child = child->sibling;
                
                // Fetch X_LOC
                ice.programDataPtr -= 3;
                if (child->data.type != TYPE_NUMBER) {
                    res = E_SYNTAX;
                }
                w24(ice.programDataPtr, child->data.operand.num);
                child = child->sibling;
                
                // Fetch the 9 small arguments
                for (loop = 0; loop < 9; loop++) {
                    ice.programDataPtr--;
                    if (child->data.type != TYPE_NUMBER) {
                        res = E_SYNTAX;
                    }
                    *ice.programDataPtr = child->data.operand.num;
                    child = child->sibling;
                }
                
                child = reverseNode(tmp);

                // Build a new tilemap struct in the program data
                LD_HL_IMM((uint24_t)ice.programDataPtr, TYPE_STRING);
            }
            
            // CopyData(
            else if (function2 == tCopyData) {
                uint8_t *prevProgDataPtr = ice.programDataPtr;
                
                /*****************************************************
                * Inputs:
                *  arg1: PTR to destination
                *  arg2: size in bytes of each entry
                *  arg3-argX: entries, constants
                *****************************************************/
                
                if ((res = parseNode(child)) != VALID) {
                    return res;
                }
                AnsToDE();
                res = InsertDataElements(sibling);
                LD_BC_IMM(prevProgDataPtr - ice.programDataPtr, TYPE_NUMBER);
                LDIR();
            }
            
            // LoadData(
            else if (function2 == tLoadData) {
                NODE *sibling2 = sibling->sibling;
                uint24_t sibling2Operand = sibling2->data.operand.num;
                
                /*****************************************************
                * Inputs:
                *  arg1: appvar name as string
                *  arg2: offset in appvar
                *  arg3: amount of sprites (or tilemap)
                *
                * Returns: PTR to table with sprite pointers (tilemap)
                * Returns: PTR to sprite (sprite)
                *****************************************************/

                if (siblingType != TYPE_NUMBER || sibling2->data.type != TYPE_NUMBER || !child->data.isString) {
                    return E_SYNTAX;
                }

                // Check if it's a sprite or a tilemap
                if (sibling2Operand == 3) {
                    // Copy the LoadData( routine to the data section
                    if (!ice.usedAlreadyLoadSprite) {
                        ice.programDataPtr -= 32;
                        ice.LoadSpriteAddr = (uintptr_t)ice.programDataPtr;
                        memcpy(ice.programDataPtr, LoadspriteData, 32);
                        ice.usedAlreadyLoadSprite = true;
                    }

                    // Set which offset
                    LD_HL_IMM(siblingOperand + 2, TYPE_NUMBER);
                    ProgramPtrToOffsetStack();
                    LD_ADDR_HL(ice.LoadSpriteAddr + 27);

                    LD_HL_IMM(childOperand - 1, childType);

                    // Call the right routine
                    ProgramPtrToOffsetStack();
                    CALL(ice.LoadSpriteAddr);

                    ResetAllRegs();
                }

                // It's a tilemap -.-
                else {
                    // Copy the LoadData( routine to the data section
                    if (!ice.usedAlreadyLoadTilemap) {
                        ice.programDataPtr -= 59;
                        ice.LoadTilemapAddr = (uintptr_t)ice.programDataPtr;
                        memcpy(ice.programDataPtr, LoadtilemapData, 59);
                        ice.usedAlreadyLoadTilemap = true;
                    }

                    // Set which offset
                    LD_HL_IMM(siblingOperand + 2, TYPE_NUMBER);
                    ProgramPtrToOffsetStack();
                    LD_ADDR_HL(ice.LoadTilemapAddr + 27);

                    // Set table base
                    LD_HL_IMM(prescan.freeMemoryPtr, TYPE_NUMBER);
                    prescan.freeMemoryPtr += sibling2Operand;
                    ProgramPtrToOffsetStack();
                    LD_ADDR_HL(ice.LoadTilemapAddr + 40);

                    // Set amount of sprites
                    LD_A(sibling2Operand / 3);
                    ProgramPtrToOffsetStack();
                    LD_ADDR_A(ice.LoadTilemapAddr + 45);

                    LD_HL_IMM(childOperand - 1, childType);

                    // Call the right routine
                    ProgramPtrToOffsetStack();
                    CALL(ice.LoadTilemapAddr);

                    ResetAllRegs();
                    reg.AIsNumber = true;
                    reg.AValue = 0;
                }
            }
            
            // SetBrightness(
            else if (function2 == tSetBrightness) {
                res = parseNode(child);
                AnsToA();

                LD_IMM_A(mpBlLevel);
            }
        }
        
        // det(, sum(, Compare(
        else {
            uint8_t temp = 0;
            uint8_t smallArguments = 0;
            uint8_t whichSmallArgument = 1 << (9 - amountOfArgs);
            
            if (function == tDet) {
                smallArguments = GraphxArgs[function2].smallArgs;
            } else if (function == tSum) {
                smallArguments = FileiocArgs[function2].smallArgs;
            } else {
                smallArguments = 0;
            }
            
            child = (child->sibling = reverseNode(child->sibling));
            
            while (child != NULL) {
                uint8_t *tempProgramPtr = ice.programPtr;
                
                if ((res = parseNode(child)) != VALID) {
                    break;
                }
                
                if (ice.programPtr != tempProgramPtr) {
                    // Write pea instead of lea
                    if (expr.outputIsNumber && expr.outputNumber >= IX_VARIABLES - 0x80 && expr.outputNumber <= IX_VARIABLES + 0x7F) {
                        *(ice.programPtr - 2) = 0x65;
                    } else {
                        if (smallArguments & whichSmallArgument) {
                            if (expr.outputIsNumber) {
                                ice.programPtr -= expr.SizeOfOutputNumber;
                                LD_L(expr.outputNumber);
                            } else if (expr.outputIsVariable) {
                                *(ice.programPtr - 2) = 0x6E;
                                ResetHL();
                            }
                            if (expr.outputRegister == REGISTER_A) {
                                OutputWrite2Bytes(OP_LD_L_A, OP_PUSH_HL);
                            } else {
                                PushHLDE();
                            }
                        } else {
                            PushHLDE();
                        }
                    }
                } else {
                    PushHLDE();
                }
                
                child = child->sibling;
                whichSmallArgument <<= 1;
            }
            top->child->sibling = reverseNode(top->child->sibling);
            
            if (function == tDet || function == tSum) {
                if (childOperand >= (function == tDet ? AMOUNT_OF_GRAPHX_FUNCTIONS : AMOUNT_OF_FILEIOC_FUNCTIONS)) {
                    res = E_UNKNOWN_C;
                }
                
                if (function == tDet) {
                    CALL(prescan.GraphxRoutinesStack[childOperand] - (uint24_t)ice.programData + PRGM_START);
                } else {
                    CALL(prescan.FileiocRoutinesStack[childOperand] - (uint24_t)ice.programData + PRGM_START);
                }
                
                if ((temp & 7) != amountOfArgs - 1) {
                    res = E_ARGUMENTS;
                } else if (temp & UN) {
                    res = E_UNKNOWN_C;
                }
            }

            // Get the amount of arguments, and call the function
            if (function == tDet) {
                temp = GraphxArgs[function2].retRegAndArgs;
                CALL(prescan.GraphxRoutinesStack[function2] - (uint24_t)ice.programData + PRGM_START);
            } else if (function == tSum) {
                temp = FileiocArgs[function2].retRegAndArgs;
                CALL(prescan.FileiocRoutinesStack[function2] - (uint24_t)ice.programData + PRGM_START);
            } else {
                if (function == tVarOut) {
                    CALL(__strcmp);
                } else {
                    CALL(__strstr);
                }
            }
            
            while (--amountOfArgs) {
                POP_BC();
            }
            
            if (function == tDet || function == tSum) {
                if (temp & RET_A) {
                    expr.outputReturnRegister = REGISTER_A;
                } else if (temp & RET_HLs) {
                    EX_S_DE_HL();
                    expr.outputReturnRegister = REGISTER_DE;
                }
            }
            
            ResetAllRegs();
            expr.outputIsNumber = expr.outputIsVariable = expr.outputIsString = false;
            ice.modifiedIY = true;
        }
    }
    
    expr.outputRegister = expr.outputReturnRegister;
    
    return res;
}


void LoadVariableInReg(uint8_t reg2, uint8_t var) {
    if (reg2 == REGISTER_A) {
        LD_A_IND_IX_OFF(var);
    } else if (reg2 == REGISTER_BC) {
        LD_BC_IND_IX_OFF(var);
    } else if (reg2 == REGISTER_DE) {
        LD_DE_IND_IX_OFF(var);
    } else {
        LD_HL_IND_IX_OFF(var);
    }
}

void LoadValueInReg(uint8_t reg2, uint24_t val, uint8_t type) {
    if (reg2 == REGISTER_A) {
        LD_A(val);
    } else if (reg2 == REGISTER_BC) {
        LD_BC_IMM(val, type);
    } else if (reg2 == REGISTER_DE) {
        LD_DE_IMM(val, type);
    } else {
        LD_HL_IMM(val, type);
    }
}

void AnsToReg(uint8_t reg2) {
    if (reg2 == REGISTER_A) {
        AnsToA();
    } else if (reg2 == REGISTER_BC) {
        AnsToBC();
    } else if (reg2 == REGISTER_DE) {
        AnsToDE();
    } else {
        AnsToHL();
    }
}

uint8_t parseFunction2Args(NODE *child, NODE *sibling, uint8_t outputReturnRegister, bool orderDoesMatter) {
    uint8_t  childType      = child->data.type;
    uint8_t  siblingType    = sibling->data.type;
    uint24_t childOperand   = child->data.operand.num;
    uint24_t siblingOperand = sibling->data.operand.num;
    uint8_t res = VALID;

    if (childType <= TYPE_STRING) {
        if (siblingType <= TYPE_STRING) {
            LD_HL_IMM(childOperand, childType);
            LoadValueInReg(outputReturnRegister, siblingOperand, siblingType);
        } else if (siblingType == TYPE_VARIABLE) {
            LD_HL_IMM(childOperand, childType);
            LoadVariableInReg(outputReturnRegister, siblingOperand);
        } else {
            if (orderDoesMatter) {
                res = parseNode(sibling);
                AnsToReg(outputReturnRegister);
                LD_HL_IMM(childOperand, childType);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(childOperand, childType);
                } else {
                    LD_HL_IMM(childOperand, childType);
                }
            }
        }
    } else if (childType == TYPE_VARIABLE) {
        if (siblingType <= TYPE_STRING) {
            if (orderDoesMatter) {
                LD_HL_IND_IX_OFF(childOperand);
                LoadValueInReg(outputReturnRegister, siblingOperand, siblingType);
            } else {
                LD_HL_IMM(siblingOperand, siblingType);
                LD_DE_IND_IX_OFF(childOperand);
            }
        } else if (siblingType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(childOperand);
            LoadVariableInReg(outputReturnRegister, siblingOperand);
        } else {
            if (orderDoesMatter) {
                res = parseNode(sibling);
                AnsToReg(outputReturnRegister);
                LD_HL_IND_IX_OFF(childOperand);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IND_IX_OFF(childOperand);
                } else {
                    LD_HL_IND_IX_OFF(childOperand);
                }
            }
        }
    } else {
        res = parseNode(child);
        if (siblingType <= TYPE_STRING) {
            AnsToHL();
            if (orderDoesMatter) {
                LoadValueInReg(outputReturnRegister, siblingOperand, siblingType);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(siblingOperand, siblingType);
                } else {
                    LD_HL_IMM(siblingOperand, siblingType);
                }
            }
        } else if (siblingType == TYPE_VARIABLE) {
            AnsToHL();
            if (orderDoesMatter) {
                LoadVariableInReg(outputReturnRegister, siblingOperand);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IND_IX_OFF(siblingOperand);
                } else {
                    LD_HL_IND_IX_OFF(siblingOperand);
                }
            }
        } else {
            if (res != VALID) {
                return res;
            }
            PushHLDE();
            res = parseNode(sibling);
            if (orderDoesMatter) {
                AnsToReg(outputReturnRegister);
                POP_HL();
            } else {
                MaybeAToHL();
                if (expr.outputRegister == REGISTER_HL) {
                    POP_DE();
                } else {
                    POP_HL();
                }
            }
        }
    }

    return res;
}

uint8_t InsertDataElements(NODE *top) {
    uint8_t size = top->data.operand.num;
    NODE *tmp = top;
    
    top = (top->sibling = reverseNode(top->sibling));
    
    while (top != NULL) {
        uint24_t operand = top->data.operand.num;
        
        if (top->data.type != TYPE_NUMBER) {
            return E_SYNTAX;
        }
        
        ice.programDataPtr -= size;
        memset(ice.programDataPtr, 0, size);
        
        if (size == 1) {
            *ice.programDataPtr = operand;
        } else if (size == 2) {
            *(uint16_t*)ice.programDataPtr = operand;
        } else {
            w24(ice.programDataPtr, operand);
        }
        
        top = top->sibling;
    }
    
    LD_HL_IMM((uint24_t)ice.programDataPtr, TYPE_STRING);
    top->sibling = reverseNode(top->sibling);
    
    return VALID;
}

void loadGetKeyFastData1(void) {
    uint8_t mem[] = {OP_AND_A, 7, OP_LD_B_A, OP_LD_A, 1, OP_JR_Z, 3, OP_ADD_A_A, OP_DJNZ, -3, OP_LD_C_A, 0};
    
    OutputWriteMem(mem);
    ResetBC();
}

void loadGetKeyFastData2(void) {
    uint8_t mem[] = {0xCB, 0x3F, 0xCB, 0x3F, OP_AND_A, 14, OP_LD_D_A, OP_LD_A, 30, OP_SUB_A_D, OP_LD_B_A, 0};
    
    OutputWriteMem(mem);
    ResetDE();
    ResetBC();
}

void InsertMallocRoutine(void) {
    bool boolUsed = ice.usedAlreadyMalloc;
    
    CallRoutine(&ice.usedAlreadyMalloc, &ice.MallocAddr, (uint8_t*)MallocData, SIZEOF_MALLOC_DATA);
    w24((uint8_t*)ice.MallocAddr + 1, prescan.freeMemoryPtr);

    if (!boolUsed) {
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.MallocAddr + 6);
        w24((uint8_t*)ice.MallocAddr + 6, ice.MallocAddr + 1);
    }

    ResetHL();
    ResetDE();
    reg.BCIsVariable = false;
    reg.BCIsNumber = true;
    reg.BCValue = 0xD13EC5;
}
