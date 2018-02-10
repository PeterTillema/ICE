#include "defines.h"
#include "functions.h"

#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "routines.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(Sqrt, "src/asm/sqrt.bin");
INCBIN(Mean, "src/asm/mean.bin");
INCBIN(Rand, "src/asm/rand.bin");
INCBIN(Timer, "src/asm/timer.bin");
INCBIN(Malloc, "src/asm/malloc.bin");
INCBIN(Sincos, "src/asm/sincos.bin");
INCBIN(Keypad, "src/asm/keypad.bin");
INCBIN(Loadsprite, "src/asm/loadsprite.bin");
INCBIN(Loadtilemap, "src/asm/loadtilemap.bin");
#endif

#ifdef SC
extern const uint8_t SqrtData[];
extern const uint8_t MeanData[];
extern const uint8_t RandData[];
extern const uint8_t TimerData[];
extern const uint8_t MallocData[];
extern const uint8_t SincosData[];
extern const uint8_t KeypadData[];
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

const uint8_t GraphxArgs[] = {
    RET_NONE | 0, ARG_NORM,    // Begin
    RET_NONE | 0, ARG_NORM,    // End
    RET_A    | 1, SMALL_1,     // SetColor
    RET_NONE | 0, ARG_NORM,    // SetDefaultPalette
    RET_NONE | 3, ARG_NORM,    // SetPalette
    RET_NONE | 1, SMALL_1,     // FillScreen
    RET_NONE | 2, SMALL_2,     // SetPixel
    RET_A    | 2, SMALL_2,     // GetPixel
    RET_A    | 0, ARG_NORM,    // GetDraw
    RET_NONE | 1, SMALL_1,     // SetDraw
    RET_NONE | 0, ARG_NORM,    // SwapDraw
    RET_NONE | 1, SMALL_1,     // Blit
    RET_NONE | 3, SMALL_123,   // BlitLines
    RET_NONE | 5, SMALL_13,    // BlitArea
    RET_NONE | 1, SMALL_1,     // PrintChar
    RET_NONE | 2, SMALL_2,     // PrintInt
    RET_NONE | 2, SMALL_2,     // PrintUInt
    RET_NONE | 1, ARG_NORM,    // PrintString
    RET_NONE | 3, ARG_NORM,    // PrintStringXY
    RET_NONE | 2, ARG_NORM,    // SetTextXY
    RET_A    | 1, SMALL_1,     // SetTextBGColor
    RET_A    | 1, SMALL_1,     // SetTextFGColor
    RET_A    | 1, SMALL_1,     // SetTextTransparentColor
    RET_NONE | 1, ARG_NORM,    // SetCustomFontData
    RET_NONE | 1, ARG_NORM,    // SetCustomFontSpacing
    RET_NONE | 1, SMALL_1,     // SetMonoSpaceFont
    RET_HL   | 1, ARG_NORM,    // GetStringWidth
    RET_HL   | 1, SMALL_1,     // GetCharWidth
    RET_HL   | 0, ARG_NORM,    // GetTextX
    RET_HL   | 0, ARG_NORM,    // GetTextY
    RET_NONE | 4, ARG_NORM,    // Line
    RET_NONE | 3, ARG_NORM,    // HorizLine
    RET_NONE | 3, ARG_NORM,    // VertLine
    RET_NONE | 3, ARG_NORM,    // Circle
    RET_NONE | 3, ARG_NORM,    // FillCircle
    RET_NONE | 4, ARG_NORM,    // Rectangle
    RET_NONE | 4, ARG_NORM,    // FillRectangle
    RET_NONE | 4, SMALL_24,    // Line_NoClip
    RET_NONE | 3, SMALL_2,     // HorizLine_NoClip
    RET_NONE | 3, SMALL_2,     // VertLine_NoClip
    RET_NONE | 3, SMALL_2,     // FillCircle_NoClip
    RET_NONE | 4, SMALL_24,    // Rectangle_NoClip
    RET_NONE | 4, SMALL_24,    // FillRectangle_NoClip
    RET_NONE | 4, ARG_NORM,    // SetClipRegion
    RET_A    | 1, ARG_NORM,    // GetClipRegion
    RET_NONE | 1, SMALL_1,     // ShiftDown
    RET_NONE | 1, SMALL_1,     // ShiftUp
    RET_NONE | 1, SMALL_1,     // ShiftLeft
    RET_NONE | 1, SMALL_1,     // ShiftRight
    RET_NONE | 3, ARG_NORM,    // Tilemap
    RET_NONE | 3, ARG_NORM,    // Tilemap_NoClip
    RET_NONE | 3, ARG_NORM,    // TransparentTilemap
    RET_NONE | 3, ARG_NORM,    // TransparentTilemap_NoClip
    RET_HL   | 3, ARG_NORM,    // TilePtr
    RET_HL   | 3, SMALL_23,    // TilePtrMapped
    UN       | 0, ARG_NORM,    // LZDecompress
    UN       | 0, ARG_NORM,    // AllocSprite
    RET_NONE | 3, ARG_NORM,    // Sprite
    RET_NONE | 3, ARG_NORM,    // TransparentSprite
    RET_NONE | 3, SMALL_3,     // Sprite_NoClip
    RET_NONE | 3, SMALL_3,     // TransparentSprite_NoClip
    RET_HL   | 3, ARG_NORM,    // GetSprite
    RET_NONE | 5, SMALL_345,   // ScaledSprite_NoClip
    RET_NONE | 5, SMALL_345,   // ScaledTransparentSprite_NoClip
    RET_HL   | 2, ARG_NORM,    // FlipSpriteY
    RET_HL   | 2, ARG_NORM,    // FlipSpriteX
    RET_HL   | 2, ARG_NORM,    // RotateSpriteC
    RET_HL   | 2, ARG_NORM,    // RotateSpriteCC
    RET_HL   | 2, ARG_NORM,    // RotateSpriteHalf
    RET_NONE | 2, ARG_NORM,    // Polygon
    RET_NONE | 2, ARG_NORM,    // Polygon_NoClip
    RET_NONE | 6, ARG_NORM,    // FillTriangle
    RET_NONE | 6, ARG_NORM,    // FillTriangle_NoClip
    UN       | 0, ARG_NORM,    // LZDecompressSprite
    RET_NONE | 2, SMALL_12,    // SetTextScale
    RET_A    | 1, SMALL_1,     // SetTransparentColor
    RET_NONE | 0, ARG_NORM,    // ZeroScreen
    RET_NONE | 1, SMALL_1,     // SetTextConfig
    RET_HL   | 1, SMALL_1,     // GetSpriteChar
    RET_HLs  | 2, SMALL_2,     // Lighten
    RET_HLs  | 2, SMALL_2,     // Darken
    RET_A    | 1, SMALL_1,     // SetFontHeight
    RET_HL   | 2, ARG_NORM,    // ScaleSprite
    RET_NONE | 3, SMALL_23,    // FloodFill
    RET_NONE | 3, ARG_NORM,    // RLETSprite
    RET_NONE | 3, SMALL_3,     // RLETSprite_NoClip
    RET_HL   | 2, ARG_NORM,    // ConvertFromRLETSprite
    RET_HL   | 2, ARG_NORM,    // ConvertToRLETSprite
    UN       | 2, ARG_NORM,    // ConvertToNewRLETSprite
    RET_HL   | 4, SMALL_34,    // RotateScaleSprite
    RET_HL   | 5, SMALL_345,   // RotatedScaledTransparentSprite_NoClip
    RET_HL   | 5, SMALL_345,   // RotatedScaledSprite_NoClip
};

const uint8_t FileiocArgs[] = {
    RET_NONE | 0, ARG_NORM,    // CloseAll
    RET_A    | 2, ARG_NORM,    // Open
    RET_A    | 3, SMALL_3,     // OpenVar
    RET_NONE | 1, SMALL_1,     // Close
    RET_HL   | 4, SMALL_4,     // Write
    RET_HL   | 4, SMALL_4,     // Read
    RET_HL   | 1, SMALL_1,     // GetChar
    RET_HL   | 2, SMALL_12,    // PutChar
    RET_HL   | 1, ARG_NORM,    // Delete
    RET_HL   | 2, SMALL_2,     // DeleteVar
    RET_HL   | 3, SMALL_23,    // Seek
    RET_HL   | 2, SMALL_2,     // Resize
    RET_HL   | 1, SMALL_1,     // IsArchived
    RET_NONE | 2, SMALL_12,    // SetArchiveStatus
    RET_HL   | 1, SMALL_1,     // Tell
    RET_HL   | 1, SMALL_1,     // Rewind
    RET_HL   | 1, SMALL_1,     // GetSize
    RET_HL   | 3, ARG_NORM,    // GetTokenString
    RET_HL   | 1, SMALL_1,     // GetDataPtr
    RET_HL   | 2, ARG_NORM,    // Detect
    RET_HL   | 3, SMALL_3,     // DetectVar
};

extern uint8_t outputStack[4096];

uint8_t parseFunction(uint24_t index) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev, *outputCurr, *outputPrevPrev, *outputPrevPrevPrev;
    uint8_t function, function2, amountOfArguments, temp, a, outputPrevType, outputPrevPrevType, res;
    uint24_t output, endIndex, startIndex, outputPrevOperand;
    
    outputPrevPrevPrev = &outputPtr[getIndexOffset(-4)];
    outputPrevPrev     = &outputPtr[getIndexOffset(-3)];
    outputPrev         = &outputPtr[getIndexOffset(-2)];
    output             = (&outputPtr[index])->operand;
    outputCurr         = &outputPtr[getIndexOffset(-1)];
    function           = output;
    function2          = output >> 16;
    amountOfArguments  = output >> 8;
    
    outputPrevOperand  = outputPrev->operand;
    outputPrevType     = outputPrev->type & 0x7F;
    outputPrevPrevType = outputPrevPrev->type & 0x7F;
    
    if (function != tNot) {
        ClearAnsFlags();
    }
    
    // rand
    if (function == tRand) {
        ProgramPtrToOffsetStack();
        CALL(ice.randAddr + 17);
        
        ice.modifiedIY = true;
        ResetAllRegs();
    }
    
    // startTmr
    else if (function == tExtTok && function2 == tStartTmr) {
        CallRoutine(&ice.usedAlreadyTimer, &ice.TimerAddr, (uint8_t*)TimerData, SIZEOF_TIMER_DATA);
    }
    
    // checkTmr(
    else if (function == tExtTok && function2 == tCheckTmr) {
        if (outputPrevType == TYPE_NUMBER) {
            LD_DE_IMM(outputPrevOperand);
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_DE_IND_IX_OFF(outputPrevOperand);
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            AnsToDE();
        } else {
            return E_SYNTAX;
        }
        
        LD_HL_IND(0xF20000);
        OR_A_SBC_HL_DE();
    }
    
    // getKey / getKey(X)
    else if (function == tGetKey) {
        if (amountOfArguments) {
            if (outputPrevType == TYPE_NUMBER) {
                uint8_t key = outputPrevOperand;
                uint8_t keyBit = 1;
                /* This is the same as
                    ((key-1)/8 & 7) * 2 = 
                    (key-1)/4 & (7*2) = 
                    (key-1) >> 2 & 14 
                */
                LD_B(0x1E - (((key - 1) >> 2) & 14));
                
                // Get the right bit for the keypress
                if ((key - 1) & 7) {
                    uint8_t a;
                    
                    for (a = 0; a < ((key - 1) & 7); a++) {
                        keyBit = keyBit << 1;
                    }
                }
                
                LD_C(keyBit);
            } else if (outputPrevType == TYPE_VARIABLE) {
                LD_A_IND_IX_OFF(outputPrevOperand);
            } else if (outputPrevType != TYPE_CHAIN_ANS) {
                return E_SYNTAX;
            }
            
            if (outputPrevType != TYPE_NUMBER) {
                if (expr.outputRegister == REGISTER_A) {
                    DEC_A();
                    LD_D_A();
                    loadGetKeyFastData1();
                    LD_A_D();
                    loadGetKeyFastData2();
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
                    loadGetKeyFastData2();
                }
            }
            
            CallRoutine(&ice.usedAlreadyGetKeyFast, &ice.getKeyFastAddr, (uint8_t*)KeypadData, SIZEOF_KEYPAD_DATA);
            ResetReg(REGISTER_HL);
            ResetReg(REGISTER_A);
        } else {
            CALL(_os_GetCSC);
            ResetReg(REGISTER_HL);
            ResetReg(REGISTER_A);
            ice.modifiedIY = false;
        }
    }
    
    // not(
    else if (function == tNot) {
        if ((res = parseFunction1Arg(index, REGISTER_HL_DE, amountOfArguments)) != VALID) {
            return res;
        }
        
        if (expr.outputRegister == REGISTER_A) {
            ADD_A(255);
            SBC_A_A();
            INC_A();
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
            if (expr.outputRegister == REGISTER_HL) {
                LD_DE_IMM(-1);
            } else {
                SCF();
                SBC_HL_HL();
            }
            ADD_HL_DE();
            SBC_HL_HL_INC_HL();
            if (expr.ZeroCarryFlagRemoveAmountOfBytes) {
                bool temp = expr.AnsSetZeroFlag;
                
                expr.ZeroCarryFlagRemoveAmountOfBytes += 8 - (expr.outputRegister != REGISTER_HL);
                expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed;
                expr.AnsSetZeroFlagReversed = temp;
                temp = expr.AnsSetCarryFlag;
                expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed;
                expr.AnsSetCarryFlagReversed = temp;
            } else {
                expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed = expr.AnsSetCarryFlagReversed = false;
                expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
                expr.AnsSetCarryFlag = true;
            }
        }
    }
    
    // sqrt(
    else if (function == tSqrt) {
        if ((res = parseFunction1Arg(index, REGISTER_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
        CallRoutine(&ice.usedAlreadySqrt, &ice.SqrtAddr, (uint8_t*)SqrtData, SIZEOF_SQRT_DATA);
        ResetAllRegs();

        expr.outputReturnRegister = REGISTER_DE;
        ice.modifiedIY = true;
    }
    
    // sin(, cos(
    else if (function == tSin || function == tCos) {
        if ((res = parseFunction1Arg(index, REGISTER_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
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
    
    // min(, max(
    else if (function == tMin || function == tMax) {
        if ((res = parseFunction2Args(index, REGISTER_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        
        OR_A_SBC_HL_DE();
        ADD_HL_DE();
        if (function == tMin) {
            JR_C(1);
        } else {
            JR_NC(1);
        }
        EX_DE_HL();
        ResetReg(REGISTER_HL);                 // DE is already reset because of "add hl, de \ ex de, hl"
    }
    
    // mean(
    else if (function == tMean) {
        if ((res = parseFunction2Args(index, REGISTER_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        
        CallRoutine(&ice.usedAlreadyMean, &ice.MeanAddr, (uint8_t*)MeanData, SIZEOF_MEAN_DATA);
        ResetReg(REGISTER_HL);
    }
    
    // remainder(
    else if (function == tExtTok && function2 == tRemainder) {
        if (outputPrevType == TYPE_NUMBER && outputPrevOperand <= 256 && !((uint8_t)outputPrevOperand & (uint8_t)(outputPrevOperand - 1))) {
            if (outputPrevPrevType == TYPE_VARIABLE) {
                LD_A_IND_IX_OFF(outputPrevPrev->operand);
            } else if (outputPrevPrevType == TYPE_CHAIN_ANS) {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_A_L();
                } else if (expr.outputRegister == REGISTER_DE) {
                    LD_A_E();
                }
            } else {
                return E_SYNTAX;
            }
            if (outputPrev->operand == 256) {
                OR_A_A();
            } else {
                AND_A(outputPrev->operand - 1);
            }
            SBC_HL_HL();
            LD_L_A();
            reg.HLIsNumber = reg.AIsNumber;
            reg.HLIsVariable = false;
            reg.HLValue = reg.AValue;
            reg.HLVariable = reg.AVariable;
        } else {
            if ((res = parseFunction2Args(index, REGISTER_BC, amountOfArguments, true)) != VALID) {
                return res;
            }
            CALL(__idvrmu);
            ResetReg(REGISTER_HL);
            ResetReg(REGISTER_DE);
            reg.AIsNumber = true;
            reg.AIsVariable = false;
            reg.AValue = 0;
        }
    }
    
    // randInt(
    else if (function == t2ByteTok && function2 == tRandInt) {
        bool usedAlreadyRand = ice.usedAlreadyRand;
        
        if (outputPrevPrevType == TYPE_STRING || outputPrevType == TYPE_STRING) {
            return E_SYNTAX;
        }
        
        if (outputPrevType == TYPE_CHAIN_ANS) {
            AnsToHL();
            if (outputPrevPrevType == TYPE_CHAIN_PUSH) {
                POP_DE();
            } else if (outputPrevPrevType == TYPE_NUMBER) {
                LD_DE_IMM(outputPrevPrev->operand - 1);
            }
        }
        if (outputPrevPrevType == TYPE_CHAIN_ANS) {
            AnsToDE();
            if (outputPrevType == TYPE_VARIABLE) {
                LD_HL_IND_IX_OFF(outputPrevOperand);
            }
        }
        if (outputPrevPrevType == TYPE_VARIABLE) {
            LD_DE_IND_IX_OFF(outputPrevPrev->operand);
        }
        if (outputPrevType == TYPE_VARIABLE && outputPrevPrevType <= TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevOperand);
        }
        if (outputPrevPrevType == TYPE_NUMBER && outputPrevType != TYPE_NUMBER) {
            LD_DE_IMM(outputPrevPrev->operand - 1);
        }
        if (outputPrevType == TYPE_NUMBER && outputPrevPrevType != TYPE_NUMBER) {
            LD_HL_IMM(outputPrevOperand + 1);
        }
        
        if (outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
            PUSH_DE();
            OR_A_SBC_HL_DE();
        }
        if (outputPrevPrevType != TYPE_NUMBER && outputPrevType != TYPE_NUMBER) {
            INC_HL();
        }
        if (outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
            PUSH_HL();
        }
        
        ProgramPtrToOffsetStack();
        CALL(ice.randAddr + 17);
        
        if (outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
            POP_BC();
        } else {
            LD_BC_IMM(outputPrevOperand - outputPrevPrev->operand + 1);
        }
        CALL(__idvrmu);
        if (outputPrevType != TYPE_NUMBER || outputPrevPrevType != TYPE_NUMBER) {
            POP_DE();
        } else {
            LD_DE_IMM(outputPrevPrev->operand);
        }
        if (outputPrevType == TYPE_NUMBER && outputPrevPrevType != TYPE_NUMBER) {
            INC_DE();
        }
        ADD_HL_DE();
        
        ice.modifiedIY = true;
        ResetReg(REGISTER_HL);
        ResetReg(REGISTER_DE);
        reg.AIsNumber = true;
        reg.AIsVariable = false;
        reg.AValue = 0;
    }
    
    // sub(
    else if (function == t2ByteTok && function2 == tSubStrng) {
        uint24_t outputPrevPrevPrevOperand = outputPrevPrevPrev->operand;
        
        // First argument should be a string
        if (outputPrevPrevPrev->type < TYPE_STRING) {
            return E_SYNTAX;
        }
        
        // Parse last 2 argument
        if ((res = parseFunction2Args(index, REGISTER_BC, amountOfArguments - 1, true)) != VALID) {
            return res;
        }
        
        // Get the string into DE
        if (outputPrevPrevPrev->type == TYPE_STRING && 
              outputPrevPrevPrevOperand != ice.tempStrings[TempString1] && outputPrevPrevPrevOperand != ice.tempStrings[TempString2]) {
            ProgramPtrToOffsetStack();
        }
        LD_DE_IMM(outputPrevPrevPrevOperand);
        
        // Add the offset to the string, and copy!
        ADD_HL_DE();
        if (outputPrevPrevPrevOperand == ice.tempStrings[TempString1]) {
            LD_DE_IMM(ice.tempStrings[TempString2]);
        } else {
            LD_DE_IMM(ice.tempStrings[TempString1]);
        }
        LDIR();
        EX_DE_HL();
        LD_HL_VAL(0);
    }
    
    // length(
    else if (function == t2ByteTok && function2 == tLength) {
        if (outputPrevType == TYPE_STRING) {
            LD_HL_STRING(outputPrev->operand, TYPE_STRING);
        } else {
            if ((res = parseFunction1Arg(index, REGISTER_HL_DE, amountOfArguments)) != VALID) {
                return res;
            }
            MaybeAToHL();
        }
        
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
        ResetReg(REGISTER_HL);
    }
    
    // Alloc(
    else if (function == tVarOut && function2 == tAlloc) {
        if ((res = parseFunction1Arg(index, REGISTER_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
        InsertMallocRoutine();
    }
    
    // Here are coming the special functions, with abnormal arguments
    
    // DefineTilemap(
    else if (function == tVarOut && function2 == tDefineTilemap) {
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
        
        uint24_t startIndex = -1 - amountOfArguments;
        uint8_t *tempDataPtr = ice.programDataPtr - 18;                 // 18 = sizeof(tilemap_t)
        element_t *outputTemp;
        uint8_t a;
        
        if (amountOfArguments < 11 || amountOfArguments > 12) {
            return E_ARGUMENTS;
        }
        
        ice.programDataPtr -= 18;
        
        // Fetch the 8 uint8_t variables
        for (a = 0; a < 9; a++) {
            outputTemp = &outputPtr[getIndexOffset(startIndex + a)];
            if (outputTemp->type != TYPE_NUMBER) {
                return E_SYNTAX;
            }
            *(tempDataPtr + a + 6) = outputTemp->operand;
        }
        
        // Fetch the only uint24_t variable (X_LOC)
        outputTemp = &outputPtr[getIndexOffset(startIndex + 9)];
        if (outputTemp->type != TYPE_NUMBER) {
            return E_SYNTAX;
        }
        *(uint24_t*)(tempDataPtr + 15) = outputTemp->operand;
        
        // Fetch the tiles/sprites
        outputTemp = &outputPtr[getIndexOffset(startIndex + 10)];
        if (outputTemp->type != TYPE_VARIABLE) {
            return E_SYNTAX;
        }
        
        LD_HL_IND_IX_OFF(outputTemp->operand);
        ProgramPtrToOffsetStack();
        LD_ADDR_HL((uint24_t)tempDataPtr + 3);
        
        // Fetch the tilemap
        if (amountOfArguments > 11) {
            ProgramPtrToOffsetStack();
            LD_HL_IMM(outputPrevOperand);
            ProgramPtrToOffsetStack();
            LD_ADDR_HL((uint24_t)tempDataPtr);
        }
        
        // Build a new tilemap struct in the program data
        ProgramPtrToOffsetStack();
        LD_HL_IMM((uint24_t)tempDataPtr);
    }
    
    // LoadData(
    else if (function == tVarOut && function2 == tLoadData) {
        /*****************************************************
        * Inputs:
        *  arg1: appvar name as string
        *  arg2: offset in appvar
        *  arg3: amount of sprites (or tilemap)
        *
        * Returns: PTR to table with sprite pointers (tilemap)
        * Returns: PTR to sprite (sprite)
        *****************************************************/
        
        if (amountOfArguments != 3 || outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
            return E_SYNTAX;
        }
        
        // Check if it's a sprite or a tilemap
        if (outputPrev->operand == 3) {
            // Copy the LoadData( routine to the data section
            if (!ice.usedAlreadyLoadSprite) {
                ice.programDataPtr -= 32;
                ice.LoadSpriteAddr = (uintptr_t)ice.programDataPtr;
                memcpy(ice.programDataPtr, LoadspriteData, 32);
                ice.usedAlreadyLoadSprite = true;
            }
            
            // Set which offset
            LD_HL_IMM(outputPrevPrev->operand + 2);
            ProgramPtrToOffsetStack();
            LD_ADDR_HL(ice.LoadSpriteAddr + 27);
            
            if (outputPrevPrevPrev->type < TYPE_STRING) {
                return E_SYNTAX;
            }
            LD_HL_STRING(outputPrevPrevPrev->operand - 1, outputPrevPrevPrev->type);
            
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
            LD_HL_IMM(outputPrevPrev->operand + 2);
            ProgramPtrToOffsetStack();
            LD_ADDR_HL(ice.LoadTilemapAddr + 27);
            
            // Set table base
            LD_HL_IMM(ice.freeMemoryPtr);
            ice.freeMemoryPtr += outputPrev->operand;
            ProgramPtrToOffsetStack();
            LD_ADDR_HL(ice.LoadTilemapAddr + 40);
            
            // Set amount of sprites
            LD_A(outputPrev->operand / 3);
            ProgramPtrToOffsetStack();
            LD_ADDR_A(ice.LoadTilemapAddr + 45);
            
            if (outputPrevPrevPrev->type < TYPE_STRING) {
                return E_SYNTAX;
            }
            LD_HL_STRING(outputPrevPrevPrev->operand - 1, outputPrevPrevPrev->type);
            
            // Call the right routine
            ProgramPtrToOffsetStack();
            CALL(ice.LoadTilemapAddr);
            
            ResetAllRegs();
            reg.AIsNumber = true;
            reg.AValue = 0;
        }
    }
    
    // Data(
    else if (function == tVarOut && function2 == tData) {
        /***********************************
        * Inputs:
        *  arg1: size in bytes of each entry
        *  arg2-argX: entries, constants
        *
        * Returns: PTR to data
        ***********************************/
        
        uint24_t startIndex = -1 - amountOfArguments;
        
        if ((res = InsertDataElements(amountOfArguments, startIndex, (&outputPtr[getIndexOffset(startIndex)])->operand, 1)) != VALID) {
            return res;
        }
    }
    
    // CopyData(
    else if (function == tVarOut && function2 == tCopyData) {
        /*****************************************************
        * Inputs:
        *  arg1: PTR to destination
        *  arg2: size in bytes of each entry
        *  arg3-argX: entries, constants
        *****************************************************/
        
        element_t *outputTemp;
        uint24_t startIndex = -1 - amountOfArguments;
        uint8_t *prevProgDataPtr = ice.programDataPtr;
        
        outputTemp = &outputPtr[getIndexOffset(startIndex)];
        if (outputTemp->type == TYPE_NUMBER) {
            LD_DE_IMM(outputTemp->operand);
        } else if (outputTemp->type == TYPE_VARIABLE) {
            LD_DE_IND_IX_OFF(outputTemp->operand);
        } else if (outputTemp->type == TYPE_CHAIN_ANS) {
            AnsToDE();
        } else {
            return E_SYNTAX;
        }
        
        if ((res = InsertDataElements(amountOfArguments, startIndex, (&outputPtr[getIndexOffset(startIndex + 1)])->operand, 2)) != VALID) {
            return res;
        }
        LD_BC_IMM(prevProgDataPtr - ice.programDataPtr);
        LDIR();
    }
    
    // Copy(
    else if (function == tVarOut && function2 == tCopy) {
        /*****************************************************
        * Inputs:
        *  arg1: PTR to destination
        *  arg2: PTR to source
        *  arg3: size in bytes
        *****************************************************/
        
        uint8_t outputPrevPrevPrevType = outputPrevPrevPrev->type & 0x7F;
        uint24_t outputPrevPrevPrevOperand = outputPrevPrevPrev->operand;
        uint24_t outputPrevPrevOperand = outputPrevPrev->operand;
        
        if (amountOfArguments < 3 || amountOfArguments > 4) {
            return E_ARGUMENTS;
        }
        
        if (outputPrevPrevPrevType <= TYPE_VARIABLE) {
            if (outputPrevPrevType == TYPE_CHAIN_PUSH) {
                if (outputPrevType != TYPE_CHAIN_ANS) {
                    return E_ICE_ERROR;
                }
                AnsToBC();
                POP_HL();
            }
            if (outputPrevPrevType == TYPE_CHAIN_ANS) {
                AnsToHL();
            }
            if (outputPrevType == TYPE_CHAIN_ANS && outputPrevPrevType != TYPE_CHAIN_PUSH) {
                AnsToBC();
            }
        } else if (outputPrevPrevPrevType == TYPE_CHAIN_ANS) {
            AnsToDE();
        } else if (outputPrevPrevPrevType == TYPE_CHAIN_PUSH) {
            if (outputPrevPrevType == TYPE_CHAIN_ANS) {
                AnsToHL();
            } else if (outputPrevType == TYPE_CHAIN_ANS) {
                PushHLDE();
                POP_BC();
            } else {
                return E_SYNTAX;
            }
            POP_DE();
        } else {
            return E_SYNTAX;
        }

        if (outputPrevPrevPrevType == TYPE_NUMBER) {
            LD_DE_IMM(outputPrevPrevPrevOperand);
        } else if (outputPrevPrevPrevType == TYPE_VARIABLE) {
            LD_DE_IND_IX_OFF(outputPrevPrevPrevOperand);
        }
        if (outputPrevPrevType == TYPE_NUMBER) {
            LD_HL_IMM(outputPrevPrevOperand);
        } else if (outputPrevPrevType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevPrevOperand);
        }
        if (outputPrevType == TYPE_NUMBER) {
            LD_BC_IMM(outputPrevOperand);
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_BC_IND_IX_OFF(outputPrevOperand);
        }
        if (amountOfArguments == 4) {
            LDDR();
        } else {
            LDIR();
        }
        
        return VALID;
    }
    
    // DefineSprite(
    else if (function == tVarOut && function2 == tDefineSprite) {
        /*****************************************************
        * Inputs:
        *  arg1: sprite width
        *  arg2: sprite height
        *  (arg3: sprite data)
        *
        * Returns: PTR to sprite
        *****************************************************/
        
        if (amountOfArguments == 2) {
            uint8_t width = outputPrevPrev->operand;
            uint8_t height = outputPrev->operand;
            
            if (outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
                return E_SYNTAX;
            }
            
            LD_HL_IMM(width * height + 2);
            InsertMallocRoutine();
            JR_NC(6);
            LD_HL_VAL(width);
            INC_HL();
            LD_HL_VAL(height);
            DEC_HL();
        } else if (amountOfArguments == 3) {
            uint8_t *a;
            
            if(outputPrevPrevPrev->type != TYPE_NUMBER || outputPrevPrevType != TYPE_NUMBER || outputPrevType != TYPE_STRING) {
                return E_SYNTAX;
            }
            
            ice.programDataPtr -= 2;
            ProgramPtrToOffsetStack();
            LD_HL_IMM((uint24_t)ice.programDataPtr);
            ResetReg(REGISTER_HL);
            
            *ice.programDataPtr = outputPrevPrevPrev->operand;
            *(ice.programDataPtr + 1) = outputPrevPrev->operand;
        } else {
            return E_ARGUMENTS;
        }
    }
    
    // SetBrightness
    else if (function == tVarOut && function2 == tSetBrightness) {
        if ((res = parseFunction1Arg(index, REGISTER_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
        if (expr.outputRegister == REGISTER_HL) {
            LD_A_L();
        } else if (expr.outputRegister == REGISTER_DE) {
            LD_A_E();
        }
        
        LD_IMM_A(mpBlLevel);
    }
    
    // {}
    else if (function == tLBrace) {
        /*****************************************************
        * Inputs:
        *  arg1: PTR
        *
        * Returns: 1-, 2- or 3-byte value at address PTR
        *****************************************************/
        
        if (amountOfArguments != 1) {
            return E_ARGUMENTS;
        }
        if (outputPrevType == TYPE_NUMBER || outputPrevType == TYPE_STRING || outputPrev->type == TYPE_OS_STRING) {
            if (outputPrevType == TYPE_STRING && outputPrevOperand != ice.tempStrings[TempString1] && outputPrevOperand != ice.tempStrings[TempString2]) {
                ProgramPtrToOffsetStack();
            }
            if (outputCurr->mask == TYPE_MASK_U8) {
                LD_A_ADDR(outputPrevOperand);
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                LD_HL_ADDR(outputPrevOperand);
                EX_S_DE_HL();
                expr.outputReturnRegister = REGISTER_DE;
            } else {
                LD_HL_ADDR(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevOperand);
            if (outputCurr->mask == TYPE_MASK_U8) {
                LD_A_HL();
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                LD_HL_HL();
                EX_S_DE_HL();
                expr.outputReturnRegister = REGISTER_DE;
            } else {
                LD_HL_HL();
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (outputCurr->mask == TYPE_MASK_U8) {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_A_HL();
                } else {
                    LD_A_DE();
                }
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                AnsToHL();
                LD_HL_HL();
                EX_S_DE_HL();
                expr.outputReturnRegister = REGISTER_DE;
            } else {
                AnsToHL();
                LD_HL_HL();
            }
        } else {
            return E_SYNTAX;
        }
        if (outputCurr->mask == TYPE_MASK_U8) {
            expr.outputReturnRegister = REGISTER_A;
        }
    }
    
    // det(, sum(
    else if (function == tDet || function == tSum) {
        /*****************************************************
        * Inputs:
        *  arg1: which det( or sum( function
        *  arg2-argX: arguments
        *
        * Returns: output of C function
        *****************************************************/
        
        uint8_t smallArguments;
        uint8_t whichSmallArgument = 1 << (9 - amountOfArguments);
        uint8_t *startProgramPtr = 0;
        
        if (function == tDet) {
            smallArguments = GraphxArgs[function2 * 2 + 1];
        } else {
            smallArguments = FileiocArgs[function2 * 2 + 1];
        }
        
        endIndex = index;
        startIndex = index;
        
        // Get all the arguments
        for (a = amountOfArguments; a >= 1; a--) {
            uint24_t *tempP1, *tempP2;
            
            a--;
            temp = 0;
            while (1) {
                outputPrev = &outputPtr[--startIndex];
                outputPrevType = outputPrev->type;
                outputPrevOperand = outputPrev->operand;
                
                if (outputPrevType == TYPE_C_START) {
                    if (!temp) {
                        break;
                    }
                    temp--;
                }
                
                if (outputPrevType == TYPE_FUNCTION && ((uint8_t)outputPrevOperand == tDet || (uint8_t)outputPrevOperand == tSum)) {
                    temp++;
                }
                
                if (outputPrevType == TYPE_ARG_DELIMITER && !temp) {
                    break;
                }
            }
            
            // Check if it's the first argument or not
            if ((outputPrevType == TYPE_ARG_DELIMITER && !a) ||
                (outputPrevType == TYPE_C_START && a)) {
                return E_ARGUMENTS;
            }
            
            // Setup a new stack
            tempP1 = getStackVar(0);
            tempP2 = getStackVar(1);
            ice.stackDepth++;
            
            startProgramPtr = ice.programPtr;
            
            // And finally grab the argument, and return if an error occured
            if ((temp = parsePostFixFromIndexToIndex(startIndex + 1, endIndex - 1)) != VALID) {
                return temp;
            }
            
            // If the last (first) argument is fetched, it's the det( function, so ignore all the optimizations
            // Ignore them too if it's optimized, like fetching variable A if it's already in register HL
            if (ice.programPtr != startProgramPtr && a) {
                if (expr.outputIsNumber && expr.outputNumber >= IX_VARIABLES - 0x80 && expr.outputNumber <= IX_VARIABLES + 0x7F) {
                    *(ice.programPtr - 2) = 0x65;
                } else {
                    if (smallArguments & whichSmallArgument) {
                        if (expr.outputIsNumber) {
                            ice.programPtr -= expr.SizeOfOutputNumber;
                            LD_L(expr.outputNumber);
                            ResetReg(REGISTER_HL);
                        } else if (expr.outputIsVariable) {
                            *(ice.programPtr - 2) = 0x6E;
                            ResetReg(REGISTER_HL);
                        }
                        if (expr.outputRegister == REGISTER_A) {
                            LD_L_A();
                            PUSH_HL();
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
            
            ice.stackDepth--;
            
            // And restore the stack
            setStackValues(tempP1, tempP2);
            
            endIndex = startIndex;
            whichSmallArgument <<= 1;
            a++;
        }
        
        ice.programPtr = startProgramPtr;
        
        // Wow, unknown C function?
        if (function2 >= (function == tDet ? AMOUNT_OF_GRAPHX_FUNCTIONS : AMOUNT_OF_FILEIOC_FUNCTIONS)) {
            return E_UNKNOWN_C;
        }
        
        // Get the amount of arguments, and call the function
        if (function == tDet) {
            temp = GraphxArgs[function2 * 2];
            CALL((uint24_t)ice.CBaseAddress + ice.GraphxRoutinesStack[function2]);
        } else {
            temp = FileiocArgs[function2 * 2];
            CALL((uint24_t)ice.CBaseAddress + ice.FileiocRoutinesStack[function2]);
        }
        
        // Check if unimplemented function
        if (temp & UN) {
            return E_UNKNOWN_C;
        }
        
        // Check the right amount of arguments
        if ((temp & 7) != amountOfArguments - 1) {
            return E_ARGUMENTS;
        }
        
        // And pop the arguments
        for (a = 1; a < amountOfArguments; a++) {
            POP_BC();
        }
        
        // Check if the output is 16-bits OR in A
        if (temp & RET_A) {
            expr.outputReturnRegister = REGISTER_A;
        } else if (temp & RET_HLs) {
            EX_S_DE_HL();
            expr.outputReturnRegister = REGISTER_DE;
        }
        
        // Warn if C function used BEFORE starting GRAPHX
        if (function == tDet) {
            if (function2 == 1) {
                ice.endedGRAPHX = true;
            }
            if (!function2) {
                ice.startedGRAPHX = true;
            }
            if (!ice.startedGRAPHX) {
                displayError(W_START_GRAPHX);
            }
            ice.startedGRAPHX = true;
        }
        
        // Warn if C function used BEFORE starting FILEIOC
        else {
            if (!function2) {
                ice.startedFILEIOC = true;
            }
            if (!ice.startedFILEIOC) {
                displayError(W_START_FILEIOC);
            }
            ice.startedFILEIOC = true;
        }
        
        ResetAllRegs();
        expr.outputIsNumber = expr.outputIsVariable = expr.outputIsString = false;
        ice.modifiedIY = true;
    }
    
    expr.outputRegister = expr.outputReturnRegister;
    
    return VALID;
}

uint8_t parseFunction1Arg(uint24_t index, uint8_t outputRegister1, uint8_t amountOfArguments) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev;
    uint24_t outputOperand;
    uint8_t outputPrevType;
    
    outputPrev = &outputPtr[getIndexOffset(-2)];
    outputPrevType = outputPrev->type;
    outputOperand = outputPrev->operand;
    
    if (amountOfArguments != 1) {
        return E_ARGUMENTS;
    }
    
    if ((outputPrevType & 0x7F) == TYPE_NUMBER) {
        LD_HL_IMM(outputOperand);
    } else if (outputPrevType == TYPE_VARIABLE) {
        LD_HL_IND_IX_OFF(outputOperand);
    } else if (outputPrevType == TYPE_CHAIN_ANS) {
        if (outputRegister1 == REGISTER_HL) {
            AnsToHL();
        }
    } else {
        return E_SYNTAX;
    }
    
    return VALID;
}

uint8_t parseFunction2Args(uint24_t index, uint8_t outputReturnRegister, uint8_t amountOfArguments, bool orderDoesMatter) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev, *outputPrevPrev;
    uint8_t outputPrevType, outputPrevPrevType;
    uint24_t outputPrevOperand, outputPrevPrevOperand;
    
    outputPrev            = &outputPtr[getIndexOffset(-2)];
    outputPrevPrev        = &outputPtr[getIndexOffset(-3)];
    outputPrevType        = outputPrev->type;
    outputPrevPrevType    = outputPrevPrev->type;
    outputPrevOperand     = outputPrev->operand;
    outputPrevPrevOperand = outputPrevPrev->operand;
    
    if (amountOfArguments != 2) {
        return E_ARGUMENTS;
    }
    
    if ((outputPrevPrevType & 0x7F) == TYPE_NUMBER) {
        if ((outputPrevType & 0x7F) == TYPE_NUMBER) {
            LD_HL_IMM(outputPrevPrevOperand);
            if (outputReturnRegister == REGISTER_BC) {
                LD_BC_IMM(outputPrevOperand);
            } else {
                LD_DE_IMM(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_IMM(outputPrevPrevOperand);
            if (outputReturnRegister == REGISTER_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                if (outputReturnRegister == REGISTER_DE) {
                    AnsToDE();
                } else {
                    PushHLDE();
                    POP_BC();
                }
                LD_HL_IMM(outputPrevPrevOperand);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(outputPrevPrevOperand);
                } else {
                    LD_HL_IMM(outputPrevPrevOperand);
                }
            }
        } else {
            return E_SYNTAX;
        }
    } else if (outputPrevPrevType == TYPE_VARIABLE) {
        if ((outputPrevType & 0x7F) == TYPE_NUMBER) {
            if (orderDoesMatter) {
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
                if (outputReturnRegister == REGISTER_DE) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_BC_IMM(outputPrevOperand);
                }
            } else {
                LD_HL_IMM(outputPrevOperand);
                LD_DE_IND_IX_OFF(outputPrevPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            if (outputReturnRegister == REGISTER_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                if (outputReturnRegister == REGISTER_DE) {
                    AnsToDE();
                } else {
                    PushHLDE();
                    POP_BC();
                }
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IND_IX_OFF(outputPrevPrevOperand);
                } else {
                    LD_HL_IND_IX_OFF(outputPrevPrevOperand);
                }
            }
        } else {
            return E_SYNTAX;
        }
    } else if (outputPrevPrevType == TYPE_CHAIN_ANS) {
        if ((outputPrevType & 0x7F) == TYPE_NUMBER) {
            AnsToHL();
            if (orderDoesMatter) {
                if (outputReturnRegister == REGISTER_DE) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_BC_IMM(outputPrevOperand);
                }
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_HL_IMM(outputPrevOperand);
                }
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            AnsToHL();
            if (orderDoesMatter) {
                if (outputReturnRegister == REGISTER_DE) {
                    LD_DE_IND_IX_OFF(outputPrevOperand);
                } else {
                    LD_BC_IND_IX_OFF(outputPrevOperand);
                }
            } else {
                if (expr.outputRegister == REGISTER_HL) {
                    LD_DE_IND_IX_OFF(outputPrevOperand);
                } else {
                    LD_HL_IND_IX_OFF(outputPrevOperand);
                }
            }
        } else {
            return E_SYNTAX;
        }
    } else if (outputPrevPrevType == TYPE_CHAIN_PUSH) {
        if (outputPrevType != TYPE_CHAIN_ANS) {
            return E_ICE_ERROR;
        }
        if (orderDoesMatter) {
            if (outputReturnRegister == REGISTER_DE) {
                AnsToDE();
            } else {
                PushHLDE();
                POP_BC();
            }
            POP_HL();
        } else {
            MaybeAToHL();
            if (expr.outputRegister == REGISTER_HL) {
                POP_DE();
            } else {
                POP_HL();
            }
        }
    } else {
        return E_SYNTAX;
    }
    
    return VALID;
}

uint8_t InsertDataElements(uint8_t amountOfArguments, uint24_t startIndex, uint8_t dataSize, uint8_t startA) {
    element_t *outputTemp;
    element_t *outputPtr = (element_t*)outputStack;
    uint8_t a;
    uint8_t *newProgramDataPtr;
    
    ProgramPtrToOffsetStack();
    ice.programDataPtr -= dataSize * (amountOfArguments - startA);
    newProgramDataPtr = ice.programDataPtr;
    LD_HL_IMM((uint24_t)newProgramDataPtr);
    
    for (a = startA; a < amountOfArguments; a++) {
        outputTemp = &outputPtr[getIndexOffset(startIndex + a)];
        if (outputTemp->type != TYPE_NUMBER) {
            return E_SYNTAX;
        }
        memset(ice.programDataPtr, 0, dataSize);
        if (dataSize == 1) {
            *ice.programDataPtr = outputTemp->operand;
        } else if (dataSize == 2) {
            *(uint16_t*)ice.programDataPtr = outputTemp->operand;
        } else {
            *(uint24_t*)ice.programDataPtr = outputTemp->operand;
        }
        ice.programDataPtr += dataSize;
    }
    
    ice.programDataPtr = newProgramDataPtr;
    
    return VALID;
}

void loadGetKeyFastData1(void) {
    AND_A(7);
    LD_B_A();
    LD_A(1);
    JR_Z(3);
    ADD_A_A();
    DJNZ(-3);
    LD_C_A();
}

void loadGetKeyFastData2(void) {
    SRL_A();
    SRL_A();
    AND_A(14);
    LD_D_A();
    LD_A(30);
    SUB_A_D();
    LD_B_A();
}

void InsertMallocRoutine(void) {
    bool boolUsed = ice.usedAlreadyMalloc;
    
    CallRoutine(&ice.usedAlreadyMalloc, &ice.MallocAddr, (uint8_t*)MallocData, SIZEOF_MALLOC_DATA);
    *(uint24_t*)(ice.MallocAddr + 1) = ice.freeMemoryPtr;
    
    if (!boolUsed) {
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.MallocAddr + 6);
        *(uint24_t*)(ice.MallocAddr + 6) = ice.MallocAddr + 1;
    }
    
    ResetReg(REGISTER_HL);
    ResetReg(REGISTER_DE);
    reg.BCIsVariable = false;
    reg.BCIsNumber = true;
    reg.BCValue = 0xD13EC5;
}