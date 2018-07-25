#include "defines.h"
#include "parse.h"

#include "ast.h"
#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "output.h"
#include "routines.h"
#include "prescan.h"

#ifndef CALCULATOR
extern const uint8_t PauseData[];
extern const uint8_t InputData[];
extern const uint8_t PrgmData[];
extern const uint8_t DispData[];
extern const uint8_t RandData[];

extern char *str_dupcat(const char *s, const char *c);
#endif

static uint8_t (*functions[256])(int token);
extern const function_t implementedFunctions[AMOUNT_OF_FUNCTIONS];

uint8_t parseProgram(void) {
    char buf[21];
    uint8_t currentGoto, currentLbl, ret, *randAddr, *amountOfLinesOffset = 0;
    
    LD_IX_IMM(IX_VARIABLES);
    
    _seek(0, SEEK_END, ice.inPrgm);
    if (!_tell(ice.inPrgm)) {
        return VALID;
    }
    _rewind(ice.inPrgm);
    
    // Eventually seed the rand
    if (prescan.amountOfRandRoutines) {
        ice.programDataPtr -= SIZEOF_RAND_DATA;
        randAddr = ice.programDataPtr;
        ice.randAddr = (uint24_t)randAddr;
        memcpy(ice.programDataPtr, RandData, SIZEOF_RAND_DATA);
        
        // Write internal pointers
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(randAddr + 2);
        w24(randAddr + 2, ice.randAddr + 102);
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(randAddr + 6);
        w24(randAddr + 6, ice.randAddr + 105);
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(randAddr + 19);
        w24(randAddr + 19, ice.randAddr + 102);

        // Call seed rand
        LD_HL_IND(0xF30044);
        ProgramPtrToOffsetStack();
        CALL((uint24_t)ice.programDataPtr);
    }
    
    // Open debug appvar to store things to
#ifdef CALCULATOR
    sprintf(buf, "%.5sDBG", ice.outName);
    ice.dbgPrgm = ti_Open(buf, "w");
    
    if (ice.debug) {
        uint8_t curVar;
        const uint8_t mem[] = {OP_LD_HL_IND, 0xE4, 0x25, 0xD0, 0xED, 0x17, OP_EX_DE_HL, OP_LD_BC, 0x83, OP_CP_A_A, OP_RET,
                                           OP_OR_A_A, 0xED, 0x42, OP_RET_NZ, OP_EX_DE_HL, OP_INC_HL, OP_INC_HL, OP_INC_HL, 0};
        char buf[10];
        
        OutputWriteMem(mem);
        *--ice.programDataPtr = 0;
        sprintf(buf, "%c%.5sDBG", TI_APPVAR_TYPE, ice.outName);
        ice.programDataPtr -= strlen(buf);
        strcpy((char*)ice.programDataPtr, buf);
        ProgramPtrToOffsetStack();
        LD_DE_IMM((uint24_t)ice.programDataPtr);
        
        *--ice.programDataPtr = OP_JP_HL;
        ProgramPtrToOffsetStack();
        CALL((uint24_t)ice.programDataPtr);
        
        LD_IY_IMM(flags);
        OutputWriteByte(OP_RET_C);
        ice.modifiedIY = false;
        
        ResetAllRegs();
        
        if (!ice.dbgPrgm) {
            return E_NO_DBG_FILE;
        }
        
        // Write input name to debug appvar
        sprintf(buf, "%s\x00", ice.currProgName[ice.inPrgm]);
        ti_Write(buf, strlen(buf) + 1, 1, ice.dbgPrgm);
        
        // Write FILEIOC functions pointer to debug appvar
        ti_Write(&ice.FileiocFunctionsPointer, 3, 1, ice.dbgPrgm);
        
        ti_PutC(prescan.amountOfVariablesUsed, ice.dbgPrgm);
        for (curVar = 0; curVar < prescan.amountOfVariablesUsed; curVar++) {
            sprintf(buf, "%s\x00", prescan.variables[curVar].name);
            ti_Write(buf, strlen(buf) + 1, 1, ice.dbgPrgm);
        }
        
        amountOfLinesOffset = ti_GetDataPtr(ice.dbgPrgm);
        WriteIntToDebugProg(0);
    } else if (ice.dbgPrgm) {
        ti_Delete(buf);
    }
#endif

    if ((ret = parseProgramUntilEnd()) != VALID) {
        return ret;
    }
    
    if (!ice.lastTokenIsReturn) {
        RET();
    }
    
#ifdef CALCULATOR
    if (ice.debug) {
        ti_PutC(0xFF, ice.dbgPrgm);
        w24(amountOfLinesOffset, ice.currentLine);
        
        // Write all the startup breakpoints to the debug appvar
        ti_PutC(ice.currentBreakPointLine, ice.dbgPrgm);
        for (currentLbl = 0; currentLbl < ice.currentBreakPointLine; currentLbl++) {
            WriteIntToDebugProg(ice.breakPointLines[currentLbl]);
        }
        
        // Write all the labels to the debug appvar
        ti_PutC(ice.curLbl, ice.dbgPrgm);
        
        for (currentLbl = 0; currentLbl < ice.curLbl; currentLbl++) {
            label_t *curLbl = &ice.LblStack[currentLbl];
            uint24_t addr = curLbl->addr - ice.programData + PRGM_START;
            
            ti_Write(curLbl->name, strlen(curLbl->name) + 1, 1, ice.dbgPrgm);
            WriteIntToDebugProg(addr);
        }
    }
#endif
    
    // Find all the matching Goto's/Lbl's
    for (currentGoto = 0; currentGoto < ice.curGoto; currentGoto++) {
        label_t *curGoto = &ice.GotoStack[currentGoto];

        for (currentLbl = 0; currentLbl < ice.curLbl; currentLbl++) {
            label_t *curLbl = &ice.LblStack[currentLbl];

            if (!strcmp(curLbl->name, curGoto->name)) {
                uint24_t jumpAddress = (uint24_t)curLbl->addr - (uint24_t)ice.programData + PRGM_START;
                
                w24(curGoto->addr + 1, jumpAddress);
                
                if (ice.debug) {
                    w24(curGoto->debugJumpDataPtr, jumpAddress);
                }
                
                goto findNextLabel;
            }
        }

        // Label not found
        displayLabelError(curGoto->name);
        _seek(curGoto->offset, SEEK_SET, ice.inPrgm);
        
        return W_VALID;
findNextLabel:;
    }
    
    return VALID;
}

uint8_t parseProgramUntilEnd(void) {
    int token;
    uint8_t *tempProgramPtr = ice.programData;
    
    // Do things based on the token
    while ((token = _getc()) != EOF) {
        uint8_t ret;
        uint8_t *tempProgramPtr = ice.programPtr;
        uint16_t currentOffset = 0;
        
#ifdef CALCULATOR
        if (ice.debug) {
            currentOffset = ti_Tell(ice.dbgPrgm);
            WriteIntToDebugProg((uint24_t)ice.programPtr - (uint24_t)ice.programData + PRGM_START);
            
            if ((uint8_t)token == tReturn) {
                WriteIntToDebugProg(-1);
            }
        }
#endif
        
        ice.lastTokenIsReturn = false;
        ice.currentLine++;
        
        if ((ret = (*functions[token])(token)) != VALID) {
            return ret;
        }
        
#ifdef CALCULATOR
        if (ice.debug) {
            while ((ice.programPtr > tempProgramPtr) && (ice.programPtr - tempProgramPtr < 4)) {
                OutputWriteByte(0);
            }
            
            if (ti_Tell(ice.dbgPrgm) - currentOffset == 3) {
                WriteIntToDebugProg(0);
            }
        }

        displayLoadingBar();
#endif
    }
    
    return VALID;
}

uint8_t parseExpression(int token) {
    NODE *outputNode = NULL;
    NODE *stackNode = NULL;
    uint8_t index = 0, a, stackToOutputReturn, tok, canUseMask = 2, res;
    uint24_t loopIndex, temp;
    element_t tempElement = {0};
    
    while (token != EOF && (tok = (uint8_t)token) != tEnter && tok != tColon) {
        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;

            while ((uint8_t)(token = _getc()) >= t0 && (uint8_t)token <= t9) {
                output = output * 10 + token - t0;
            }
            tempElement.type = TYPE_NUMBER;
            tempElement.operand.num = output;
            outputNode = push(outputNode, tempElement);

            // Don't grab a new token
            continue;
        }
        
        // Process a hexadecimal number
        else if (tok == tee) {
            uint24_t output = 0;

            while ((tok = IsHexadecimal(token = _getc())) != 16) {
                output = (output << 4) + tok;
            }
            tempElement.type = TYPE_NUMBER;
            tempElement.operand.num = output;
            outputNode = push(outputNode, tempElement);

            // Don't grab a new token
            continue;
        }
        
        // Process a binary number
        else if (tok == tPi) {
            uint24_t output = 0;

            while ((tok = (token = _getc())) >= t0 && tok <= t1) {
                output = (output << 1) + tok - t0;
            }
            tempElement.type = TYPE_NUMBER;
            tempElement.operand.num = output;
            outputNode = push(outputNode, tempElement);

            // Don't grab a new token
            continue;
        }
        
        // Process a 'negative' number or expression
        else if (tok == tChs) {
            if ((token = _getc()) >= t0 && token <= t9) {
                uint24_t output = token - t0;

                while ((uint8_t)(token = _getc()) >= t0 && (uint8_t)token <= t9) {
                    output = output * 10 + token - t0;
                }
                tempElement.type = TYPE_NUMBER;
                tempElement.operand.num = 0 - output;
                outputNode = push(outputNode, tempElement);

                // Don't grab a new token
                continue;
            } else {
                // Pretend as if it's a -1 * 
                tempElement.type = TYPE_NUMBER;
                tempElement.operand.num = -1;
                outputNode = push(outputNode, tempElement);
                
                SeekMinus1();
                token = tMul;
                
                // Don't grab a new token
                continue;
            }
        }
        
        // Process an OS list (number or list element)
        else if (tok == tVarLst) {
            tempElement.type = TYPE_NUMBER;
            tempElement.operand.num = prescan.OSLists[_getc()];
            outputNode = push(outputNode, tempElement);
            
            // Check if it's a list element
            if ((uint8_t)(token = _getc()) == tLParen) {
                tempElement.type = TYPE_FUNCTION;
                tempElement.operand.func.function = 0x0F;
                tempElement.operand.func.amountOfArgs = 1;
                tempElement.operand.func.index = -1;
                stackNode = push(stackNode, tempElement);
                
                token = tAdd;
            }
            
            // Dont grab a new token
            continue;
        }
        
        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            tempElement.type = TYPE_VARIABLE;
            tempElement.operand.var = GetVariableOffset(tok);
            outputNode = push(outputNode, tempElement);
        }

        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            uint8_t precedence = operatorPrecedence[index - 1];
            
            if (tok == tStore) {
                if ((outputNode = stackToOutput(outputNode, &stackNode)) == NULL) {
                    res = E_ARGUMENTS;
                    goto parseError;
                }
            }
            
            while (stackNode != NULL && stackNode->data.type == TYPE_OPERATOR && precedence <= stackNode->data.operand.op.precedence) {
                stackNode = pop(stackNode, &tempElement);
                if ((outputNode = insertData(outputNode, tempElement, 2)) == NULL) {
                    res = E_SYNTAX;
                    goto parseError;
                }
            }
            tempElement.type = TYPE_OPERATOR;
            tempElement.operand.op.op = tok;
            tempElement.operand.op.index = index;
            tempElement.operand.op.precedence = operatorPrecedence2[index - 1];
            stackNode = push(stackNode, tempElement);
        }
        
        // Gets the address of a variable
        else if (tok == tFromDeg) {
            tempElement.type = TYPE_NUMBER;
            tok = _getc();

            // Get the address of the variable
            if (tok >= tA && tok <= tTheta) {
                tempElement.operand.num = IX_VARIABLES + (char)GetVariableOffset(tok);
            } else if (tok == tVarLst) {
                tempElement.operand.num = prescan.OSLists[_getc()];
            } else if (tok == tVarStrng) {
                tempElement.operand.num = prescan.OSStrings[_getc()];
            } else {
                res = E_SYNTAX;
                goto parseError;
            }
            outputNode = push(outputNode, tempElement);
        }
        
        // ) } ,
        else if (tok == tRParen || tok == tRBrace || tok == tComma) {
            uint8_t index, function;
            
            while (stackNode != NULL && stackNode->data.type != TYPE_FUNCTION) {
                stackNode = pop(stackNode, &tempElement);
                if ((outputNode = insertData(outputNode, tempElement, 2)) == NULL) {
                    res = E_SYNTAX;
                    goto parseError;
                }
            }
            
            if (stackNode == NULL) {
                if (expr.inFunction) {
                    ice.tempToken = tok;
                    
                    goto stopParsing;
                }
                
                res = E_EXTRA_RPAREN;
                goto parseError;
            }
            
            stackNode = pop(stackNode, &tempElement);
            function = tempElement.operand.func.function;
            index = tempElement.operand.func.index;
            
            if (tok == tComma) {
                tempElement.operand.func.amountOfArgs++;
                stackNode = push(stackNode, tempElement);
            } else {
                if ((tok == tRBrace && function != tLBrace) || (tok == tRParen && function == tLBrace)) {
                    res = E_SYNTAX;
                    goto parseError;
                }
                
                if (function == 0x0F) {
                    tempElement.operand.func.function = tLBrace;
                }
                
                if ((outputNode = insertData(outputNode, tempElement, tempElement.operand.func.amountOfArgs)) == NULL) {
                    res = E_ARGUMENTS;
                    goto parseError;
                }
            }
        }
        
        // Parse an OS string
        else if (tok == tVarStrng) {
            tempElement.isString = true;
            tempElement.type = TYPE_NUMBER;
            tempElement.operand.num = prescan.OSStrings[_getc()];
            outputNode = push(outputNode, tempElement);
            tempElement.isString = false;
        }
        
        // Parse a function
        else {
            uint8_t index, tok2 = 0;

            if (IsA2ByteTok(tok)) {
                tok2 = _getc();
            }
            
            if ((index = GetIndexOfFunction(tok, tok2)) != 255) {
                tempElement.type = TYPE_FUNCTION;
                tempElement.operand.func.function = tok;
                tempElement.operand.func.function2 = tok2;
                tempElement.operand.func.index = index;
                
                if (implementedFunctions[index].amountOfArgs && !(tok == tGetKey && (uint8_t)(token = _getc()) != tLParen)) {
                    tempElement.operand.func.amountOfArgs = 1;
                    stackNode = push(stackNode, tempElement);
                } else {
                    tempElement.operand.func.amountOfArgs = 0;
                    outputNode = push(outputNode, tempElement);
                    if (tok == tGetKey) {
                        continue;
                    }
                }
            } else {
                res = E_UNIMPLEMENTED;
                goto parseError;
            }
        }

        // Yay, fetch the next token, it's great, it's true, I like it
        token = _getc();
    }

    // If the expression quits normally, rather than an argument seperator
    ice.tempToken = tEnter;
stopParsing:

    // Move stack to output
    if ((outputNode = stackToOutput(outputNode, &stackNode)) == NULL) {
        res = E_ARGUMENTS;
        goto parseError;
    }
    
    // Invalid things, like "1 1"
    if (outputNode->prev != NULL) {
        res = E_SYNTAX;
        goto parseError;
    }
    
    outputNode = optimizeNode(outputNode);
#ifndef NDEBUG
    printNodeRecursively(outputNode, 0);
#endif
    expr.outputRegister = REGISTER_HL;
    res = parseNode(outputNode);
    
parseError:
    if (outputNode != NULL) {
        while (outputNode->prev != NULL) {
            outputNode = outputNode->prev;
        }
        freeNode(outputNode);
    }
    if (stackNode != NULL) {
        while (stackNode->prev != NULL) {
            stackNode = stackNode->prev;
        }
        freeNode(stackNode);
    }
    
    return res;
}

static uint8_t functionI(int token) {
    skipLine();

    return VALID;
}

static uint8_t functionIf(int token) {
    uint8_t *IfElseAddr = NULL;
    uint8_t tempGotoElements = ice.curGoto;
    uint8_t tempLblElements = ice.curLbl;
    
    if ((token = _getc()) != EOF && token != tEnter && token != tColon) {
        uint8_t *IfStartAddr, res, *IfJumpAddr = NULL;
        uint24_t tempDataOffsetElements;
        
#ifdef CALCULATOR
        if (ice.debug) {
            IfJumpAddr = ti_GetDataPtr(ice.dbgPrgm);
            WriteIntToDebugProg(0);
        }
#endif
        
        // Parse the argument
        if ((res = parseExpression(token)) != VALID) {
            return res;
        }

        if (expr.outputIsString) {
            return E_SYNTAX;
        }

        //Check if we can optimize stuff :D
        optimizeZeroCarryFlagOutput();

        // Backup stuff
        IfStartAddr = ice.programPtr;
        tempDataOffsetElements = ice.dataOffsetElements;

        if (expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed) {
            if (expr.AnsSetCarryFlagReversed) {
                JP_NC(0);
            } else {
                JP_C(0);
            }
        } else {
            if (expr.AnsSetZeroFlagReversed) {
                JP_NZ(0);
            } else {
                JP_Z(0);
            }
        }
        res = parseProgramUntilEnd();

        // Check if we quit the program with an 'Else'
        if (res == E_ELSE) {
            bool shortElseCode;
            uint8_t *ElseJumpAddr = NULL;
            uint8_t tempGotoElements2 = ice.curGoto;
            uint8_t tempLblElements2 = ice.curLbl;
            uint24_t tempDataOffsetElements2;

            // Backup stuff
            ResetAllRegs();
            IfElseAddr = ice.programPtr;
            tempDataOffsetElements2 = ice.dataOffsetElements;

            JP(0);
            
#ifdef CALCULATOR
            if (ice.debug) {
                ElseJumpAddr = ti_GetDataPtr(ice.dbgPrgm);
                WriteIntToDebugProg(0);
                w24(IfJumpAddr, (uint24_t)ice.programPtr - (uint24_t)ice.programData + PRGM_START);
            }
#endif
            
            if ((res = parseProgramUntilEnd()) != E_END && res != VALID) {
                return res;
            }

            shortElseCode = JumpForward(IfElseAddr, ice.programPtr, tempDataOffsetElements2, tempGotoElements2, tempLblElements2);
            JumpForward(IfStartAddr, IfElseAddr + (shortElseCode ? 2 : 4), tempDataOffsetElements, tempGotoElements, tempLblElements);
            
#ifdef CALCULATOR
            if (ice.debug) {
                WriteIntToDebugProg(0);
                w24(ElseJumpAddr, (uint24_t)ice.programPtr - (uint24_t)ice.programData + PRGM_START);
            }
#endif
        }

        // Check if we quit the program with an 'End' or at the end of the input program
        else if (res == E_END || res == VALID) {
            JumpForward(IfStartAddr, ice.programPtr, tempDataOffsetElements, tempGotoElements, tempLblElements);
            
#ifdef CALCULATOR
            if (ice.debug) {
                w24(IfJumpAddr, (uint24_t)ice.programPtr - (uint24_t)ice.programData + PRGM_START);
                WriteIntToDebugProg(0);
            }
#endif
        } else {
            return res;
        }

        ResetAllRegs();

        return VALID;
    } else {
        return E_NO_CONDITION;
    }
}

static uint8_t functionElse(int token) {
    if (!CheckEOL()) {
        return E_SYNTAX;
    }
    return E_ELSE;
}

static uint8_t functionEnd(int token) {
    if (!CheckEOL()) {
        return E_SYNTAX;
    }
    return E_END;
}

static uint8_t dummyReturn(int token) {
    return VALID;
}

bool JumpForward(uint8_t *startAddr, uint8_t *endAddr, uint24_t tempDataOffsetElements, uint8_t tempGotoElements, uint8_t tempLblElements) {
    if (!ice.debug && endAddr - startAddr <= 0x80) {
        uint8_t *tempPtr = startAddr;
        uint8_t opcode = *startAddr;
        uint24_t tempForLoopSMCElements = ice.ForLoopSMCElements;
        label_t *labelPtr = ice.LblStack;
        label_t *gotoPtr = ice.GotoStack;

        *startAddr++ = opcode - 0xA2 - (opcode == 0xC3 ? 9 : 0);
        *startAddr++ = endAddr - tempPtr - 4;

        // Update pointers to data, decrease them all with 2, except pointers from data to data!
        while (ice.dataOffsetElements != tempDataOffsetElements) {
            uint8_t *tempDataOffsetStackAddr = (uint8_t*)ice.dataOffsetStack[tempDataOffsetElements];

            // Check if the pointer is in the program, not in the program data
            if (tempDataOffsetStackAddr >= ice.programData && tempDataOffsetStackAddr <= ice.programPtr) {
                ice.dataOffsetStack[tempDataOffsetElements] = (uint24_t*)(tempDataOffsetStackAddr - 2);
            }
            tempDataOffsetElements++;
        }

        // Update Goto and Lbl addresses, decrease them all with 2
        while (ice.curGoto != tempGotoElements) {
            gotoPtr[tempGotoElements].addr -= 2;
            tempGotoElements++;
        }
        while (ice.curLbl != tempLblElements) {
            labelPtr[tempLblElements].addr -= 2;
            tempLblElements++;
        }

        // Update all the For loop SMC addresses
        while (tempForLoopSMCElements--) {
            if ((uint24_t)ice.ForLoopSMCStack[tempForLoopSMCElements] > (uint24_t)startAddr) {
                *ice.ForLoopSMCStack[tempForLoopSMCElements] -= 2;
                ice.ForLoopSMCStack[tempForLoopSMCElements] = (uint24_t*)(((uint8_t*)ice.ForLoopSMCStack[tempForLoopSMCElements]) - 2);
            }
        }

        if (ice.programPtr != startAddr) {
            memmove(startAddr, startAddr + 2, ice.programPtr - startAddr);
        }
        ice.programPtr -= 2;
        
        return true;
    } else {
        w24(startAddr + 1, endAddr - ice.programData + PRGM_START);
        
        return false;
    }
}

bool JumpBackwards(uint8_t *startAddr, uint8_t whichOpcode) {
    if (ice.programPtr + 2 - startAddr <= 0x80) {
        uint8_t *tempPtr = ice.programPtr;

        OutputWriteByte(whichOpcode);
        OutputWriteByte(startAddr - 2 - tempPtr);

        return true;
    } else {
        // JR cc to JP cc
        OutputWriteByte(whichOpcode + 0xA2 + (whichOpcode == 0x18 ? 9 : 0));
        OutputWriteLong(startAddr - ice.programData + PRGM_START);

        return false;
    }
}

uint8_t *WhileRepeatCondStart = NULL;
bool WhileJumpBackwardsLarge;

static uint8_t functionWhile(int token) {
    uint24_t tempDataOffsetElements = ice.dataOffsetElements;
    uint8_t tempGotoElements = ice.curGoto;
    uint8_t tempLblElements = ice.curLbl;
    uint8_t *WhileStartAddr = ice.programPtr, res;
    uint8_t *WhileRepeatCondStartTemp = WhileRepeatCondStart;
    bool WhileJumpForwardSmall;
    uint8_t *debugProgDataPtr = NULL;

    // Basically the same as "Repeat", but jump to condition checking first
    JP(0);
    
#ifdef CALCULATOR
    if (ice.debug) {
        debugProgDataPtr = ti_GetDataPtr(ice.dbgPrgm);
        WriteIntToDebugProg(0);
    }
#endif
    
    if ((res = functionRepeat(token)) != VALID) {
        return res;
    }
    
    WhileJumpForwardSmall = JumpForward(WhileStartAddr, WhileRepeatCondStart, tempDataOffsetElements, tempGotoElements, tempLblElements);
    
#ifdef CALCULATOR
    if (ice.debug) {
        w24(debugProgDataPtr, (uint24_t)WhileRepeatCondStart - (uint24_t)ice.programData + PRGM_START);
    }
#endif
    
    WhileRepeatCondStart = WhileRepeatCondStartTemp;

    if (WhileJumpForwardSmall && WhileJumpBackwardsLarge) {
        // Now the JP at the condition points to the 2nd byte after the JR to the condition, so update that too
        w24(ice.programPtr - 3, r24(ice.programPtr - 3) - 2);
    }

    return VALID;
}

uint8_t functionRepeat(int token) {
    uint24_t tempCurrentLine, tempCurrentLine2;
    uint16_t RepeatCondStart, RepeatProgEnd;
    uint8_t *RepeatCodeStart, res;

    RepeatCondStart = _tell(ice.inPrgm);
    RepeatCodeStart = ice.programPtr;
    tempCurrentLine = ice.currentLine;

    // Skip the condition for now
    skipLine();
    ResetAllRegs();
    
#ifdef CALCULATOR
    if (ice.debug && ((uint8_t)token == tRepeat)) {
        WriteIntToDebugProg(0);
    }
#endif

    // Parse the code inside the loop
    if ((res = parseProgramUntilEnd()) != E_END && res != VALID) {
        return res;
    }

    ResetAllRegs();

    // Remind where the "End" is
    RepeatProgEnd = _tell(ice.inPrgm);
    if ((uint8_t)token == tWhile) {
        WhileRepeatCondStart = ice.programPtr;
    }

    // Parse the condition
    _seek(RepeatCondStart, SEEK_SET, ice.inPrgm);
    tempCurrentLine2 = ice.currentLine;
    ice.currentLine = tempCurrentLine;

    if ((res = parseExpression(_getc())) != VALID) {
        return res;
    }
    ice.currentLine = tempCurrentLine2;

    // And set the pointer after the "End"
    _seek(RepeatProgEnd, SEEK_SET, ice.inPrgm);

    if (expr.outputIsNumber) {
        ice.programPtr -= expr.SizeOfOutputNumber;
        if ((expr.outputNumber && (uint8_t)token == tWhile) || (!expr.outputNumber && (uint8_t)token == tRepeat)) {
            WhileJumpBackwardsLarge = !JumpBackwards(RepeatCodeStart, OP_JR);
        }
        return VALID;
    }

    if (expr.outputIsString) {
        return E_SYNTAX;
    }

    optimizeZeroCarryFlagOutput();

    if ((uint8_t)token == tWhile) {
        // Switch the flags
        bool a = expr.AnsSetZeroFlag;

        expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed;
        expr.AnsSetZeroFlagReversed = a;
        a = expr.AnsSetCarryFlag;
        expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed;
        expr.AnsSetCarryFlagReversed = a;
    }

    WhileJumpBackwardsLarge = !JumpBackwards(RepeatCodeStart, expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed ?
        (expr.AnsSetCarryFlagReversed ? OP_JR_NC : OP_JR_C) :
        (expr.AnsSetZeroFlagReversed  ? OP_JR_NZ : OP_JR_Z));
        
#ifdef CALCULATOR
    if (ice.debug) {
        WriteIntToDebugProg((uint24_t)RepeatCodeStart - (uint24_t)ice.programData + PRGM_START);
    }
#endif
    
    return VALID;
}

static uint8_t functionReturn(int token) {
    uint8_t res;

    if ((token = _getc()) == EOF || (uint8_t)token == tEnter || (uint8_t)token == tColon) {
        RET();
        ice.lastTokenIsReturn = true;
    } else if (token == tIf) {
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }

        //Check if we can optimize stuff :D
        optimizeZeroCarryFlagOutput();

        OutputWriteByte((expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed ?
            (expr.AnsSetCarryFlagReversed ? OP_RET_C : OP_RET_NC) :
            (expr.AnsSetZeroFlagReversed ? OP_RET_Z : OP_RET_NZ)));
    } else {
        return E_SYNTAX;
    }
    
    return VALID;
}

static uint8_t functionDisp(int token) {
    do {
        uint8_t res;

        if ((uint8_t)(token = _getc()) == tii) {
            if ((token = _getc()) == EOF) {
                ice.tempToken = tEnter;
            } else {
                ice.tempToken = (uint8_t)token;
            }
            CALL(_NewLine);
            goto checkArgument;
        }

        // Get the argument, and display it, based on whether it's a string or the outcome of an expression
        expr.inFunction = true;
        if ((res = parseExpression(token)) != VALID) {
            return res;
        }

        AnsToHL();
        CallRoutine(&ice.usedAlreadyDisp, &ice.DispAddr, (uint8_t*)DispData, SIZEOF_DISP_DATA);
        if (!expr.outputIsString) {
            w24(ice.programPtr - 3, r24(ice.programPtr - 3) + 13);
        }

checkArgument:
        ResetAllRegs();

        // Oops, there was a ")" after the expression
        if (ice.tempToken == tRParen) {
            return E_SYNTAX;
        }
    } while (ice.tempToken != tEnter);

    return VALID;
}

static uint8_t functionOutput(int token) {
    uint8_t res;

    // Get the first argument = column
    expr.inFunction = true;
    if ((res = parseExpression(_getc())) != VALID) {
        return res;
    }

    // Return syntax error if the expression was a string or the token after the expression wasn't a comma
    if (expr.outputIsString || ice.tempToken != tComma) {
        return E_SYNTAX;
    }

    if (expr.outputIsNumber) {
        uint8_t outputNumber = expr.outputNumber;

        ice.programPtr -= expr.SizeOfOutputNumber;
        LD_A(outputNumber);
        LD_IMM_A(curRow);

        // Get the second argument = row
        expr.inFunction = true;
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }

        // Yay, we can optimize things!
        if (expr.outputIsNumber) {
            // Output coordinates in H and L, 6 = sizeof(ld a, X \ ld (curRow), a)
            ice.programPtr -= 6 + expr.SizeOfOutputNumber;
            LD_SIS_HL((expr.outputNumber << 8) + outputNumber);
            LD_SIS_IMM_HL(curRow & 0xFFFF);
        } else {
            if (expr.outputIsVariable) {
                *(ice.programPtr - 2) = OP_LD_A_HL;
            } else if (expr.outputRegister == REGISTER_HL) {
                LD_A_L();
            } else if (expr.outputRegister == REGISTER_DE) {
                LD_A_E();
            }
            LD_IMM_A(curCol);
        }
    } else {
        if (expr.outputIsVariable) {
            *(ice.programPtr - 2) = OP_LD_A_HL;
        } else if (expr.outputRegister == REGISTER_HL) {
            LD_A_L();
        } else if (expr.outputRegister == REGISTER_DE) {
            LD_A_E();
        }
        LD_IMM_A(curRow);

        // Get the second argument = row
        expr.inFunction = true;
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }

        if (expr.outputIsNumber) {
            *(ice.programPtr - 4) = OP_LD_A;
            ice.programPtr -= 2;
        } else if (expr.outputIsVariable) {
            *(ice.programPtr - 2) = OP_LD_A_HL;
        } else if (expr.outputRegister == REGISTER_HL) {
            LD_A_L();
        } else if (expr.outputRegister == REGISTER_DE) {
            LD_A_E();
        }
        LD_IMM_A(curCol);
    }

    // Get the third argument = output thing
    if (ice.tempToken == tComma) {
        expr.inFunction = true;
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        
        AnsToHL();
        CallRoutine(&ice.usedAlreadyDisp, &ice.DispAddr, (uint8_t*)DispData, SIZEOF_DISP_DATA);
        if (!expr.outputIsString) {
            w24(ice.programPtr - 3, r24(ice.programPtr - 3) + 13);
        }
        ResetAllRegs();
    }
    
    if (ice.tempToken != tEnter && (ice.tempToken == tRParen && !CheckEOL())) {
        return E_SYNTAX;
    }

    return VALID;
}

static uint8_t functionClrHome(int token) {
    if (!CheckEOL()) {
        return E_SYNTAX;
    }
    MaybeLDIYFlags();
    CALL(_HomeUp);
    CALL(_ClrLCDFull);
    ResetAllRegs();

    return VALID;
}

static uint8_t functionFor(int token) {
    bool endPointIsNumber = false, stepIsNumber = false, reversedCond = false, smallCode;
    uint24_t endPointNumber = 0, stepNumber = 0, tempDataOffsetElements;
    uint8_t tempGotoElements = ice.curGoto;
    uint8_t tempLblElements = ice.curLbl;
    uint8_t *endPointExpressionValue = 0, *stepExpression = 0, *jumpToCond, *loopStart;
    uint8_t tok, variable, res;
    
#ifdef CALCULATOR
    uint8_t *ForJumpAddr = NULL;
    
    if (ice.debug) {
        ForJumpAddr = ti_GetDataPtr(ice.dbgPrgm);
        WriteIntToDebugProg(0);
    }
#endif

    if ((tok = _getc()) < tA || tok > tTheta) {
        return E_SYNTAX;
    }
    variable = GetVariableOffset(tok);
    expr.inFunction = true;
    if (_getc() != tComma) {
        return E_SYNTAX;
    }

    // Get the start value, followed by a comma
    if ((res = parseExpression(_getc())) != VALID) {
        return res;
    }
    if (ice.tempToken != tComma) {
        return E_SYNTAX;
    }

    // Load the value in the variable
    MaybeAToHL();
    if (expr.outputRegister == REGISTER_HL) {
        LD_IX_OFF_IND_HL(variable);
    } else {
        LD_IX_OFF_IND_DE(variable);
    }

    // Get the end value
    expr.inFunction = true;
    if ((res = parseExpression(_getc())) != VALID) {
        return res;
    }

    // If the end point is a number, we can optimize things :D
    if (expr.outputIsNumber) {
        endPointIsNumber = true;
        endPointNumber = expr.outputNumber;
        ice.programPtr -= expr.SizeOfOutputNumber;
    } else {
        AnsToHL();
        endPointExpressionValue = ice.programPtr;
        LD_ADDR_HL(0);
    }

    // Check if there was a step
    if (ice.tempToken == tComma) {
        expr.inFunction = true;

        // Get the step value
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        if (ice.tempToken == tComma) {
            return E_SYNTAX;
        }

        if (expr.outputIsNumber) {
            stepIsNumber = true;
            stepNumber = expr.outputNumber;
            ice.programPtr -= expr.SizeOfOutputNumber;
        } else {
            AnsToHL();
            stepExpression = ice.programPtr;
            LD_ADDR_HL(0);
        }
    } else {
        stepIsNumber = true;
        stepNumber = 1;
    }

    jumpToCond = ice.programPtr;
    JP(0);
    tempDataOffsetElements = ice.dataOffsetElements;
    loopStart = ice.programPtr;
    ResetAllRegs();

    // Parse the inner loop
    if ((res = parseProgramUntilEnd()) != E_END && res != VALID) {
        return res;
    }

    // First add the step to the variable, if the step is 0 we don't need this
    if (!stepIsNumber || stepNumber) {
        // ld hl, (ix+*) \ inc hl/dec hl (x times) \ ld (ix+*), hl
        // ld hl, (ix+*) \ ld de, x \ add hl, de \ ld (ix+*), hl

        LD_HL_IND_IX_OFF(variable);
        if (stepIsNumber) {
            uint8_t a = 0;

            if (stepNumber < 5) {
                for (a = 0; a < (uint8_t)stepNumber; a++) {
                    INC_HL();
                }
            } else if (stepNumber > 0xFFFFFF - 4) {
                for (a = 0; a < (uint8_t)(0-stepNumber); a++) {
                    DEC_HL();
                }
            } else {
                LD_DE_IMM(stepNumber, TYPE_NUMBER);
                ADD_HL_DE();
            }
        } else {
            w24(stepExpression + 1, ice.programPtr + PRGM_START - ice.programData + 1);
            ice.ForLoopSMCStack[ice.ForLoopSMCElements++] = (uint24_t*)(endPointExpressionValue + 1);

            LD_DE_IMM(0, TYPE_NUMBER);
            ADD_HL_DE();
        }
        LD_IX_OFF_IND_HL(variable);
    }

    smallCode = JumpForward(jumpToCond, ice.programPtr, tempDataOffsetElements, tempGotoElements, tempLblElements);
    ResetAllRegs();
    
#ifdef CALCULATOR
    if (ice.debug) {
        w24(ForJumpAddr, (uint24_t)ice.programPtr - (uint24_t)ice.programData + PRGM_START);
    }
#endif

    // If both the step and the end point are a number, the variable is already in HL
    if (!(endPointIsNumber && stepIsNumber)) {
        LD_HL_IND_IX_OFF(variable);
    }

    if (endPointIsNumber) {
        if (stepNumber < 0x800000) {
            LD_DE_IMM(endPointNumber + 1, TYPE_NUMBER);
        } else {
            LD_DE_IMM(endPointNumber, TYPE_NUMBER);
            reversedCond = true;
        }
        OR_A_A();
    } else {
        w24(endPointExpressionValue + 1, ice.programPtr + PRGM_START - ice.programData + 1);
        ice.ForLoopSMCStack[ice.ForLoopSMCElements++] = (uint24_t*)(endPointExpressionValue + 1);

        LD_DE_IMM(0, TYPE_NUMBER);
        if (stepNumber < 0x800000) {
            SCF();
        } else {
            OR_A_A();
            reversedCond = true;
        }
    }
    SBC_HL_DE();

    // Jump back to the loop
    JumpBackwards(loopStart - (smallCode ? 2 : 0), OP_JR_C - (reversedCond ? 8 : 0));
    
#ifdef CALCULATOR
    if (ice.debug) {
        WriteIntToDebugProg((uint24_t)loopStart - (uint24_t)ice.programData + PRGM_START);
    }
#endif

    return VALID;
}

static uint8_t functionPrgm(int token) {
    bool tempAlreadyUsedPrgm = ice.usedAlreadyPrgm;
    uint24_t length;
    uint8_t res;
    prog_t *outputPrgm;
        
    outputPrgm = GetProgramName();
    if ((res = outputPrgm->errorCode) != VALID) {
        free(outputPrgm);
        return res;
    }
    
    length = strlen(outputPrgm->prog);
    ice.programDataPtr -= length + 2;
    *ice.programDataPtr = TI_PRGM_TYPE;
    memcpy(ice.programDataPtr + 1, outputPrgm->prog, length + 1);
    
    ProgramPtrToOffsetStack();
    LD_HL_IMM((uint24_t)ice.programDataPtr);
    CALL(_Mov9ToOP1);
    CallRoutine(&ice.usedAlreadyPrgm, &ice.PrgmAddr, (uint8_t*)PrgmData, SIZEOF_PRGM_DATA);
    
    if (!tempAlreadyUsedPrgm) {
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.PrgmAddr + 11);
        w24((uint8_t*)ice.PrgmAddr + 11, (uint24_t)ice.PrgmAddr + 30);
    }
    
    ResetAllRegs();
    ice.modifiedIY = false;
    free(outputPrgm);

    return VALID;
}

static uint8_t functionCustom(int token) {
    uint8_t tok = _getc();

    if (tok >= tDefineSprite && tok <= tCompare) {
        // Call
        if (tok == tCall) {
            insertGotoLabel();
            CALL(0);

            return VALID;
        } else {
            SeekMinus1();

            return parseExpression(token);
        }
    } else {
        return E_UNIMPLEMENTED;
    }
}

static uint8_t functionLbl(int token) {
    // Add the label to the stack, and skip the line
    label_t *labelCurr = &ice.LblStack[ice.curLbl++];
    uint8_t a = 0;
    
    // Get the label name
    while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tColon && a < 20) {
        labelCurr->name[a++] = token;
    }
    labelCurr->name[a] = 0;
    labelCurr->addr = ice.programPtr;
    ResetAllRegs();

    return VALID;
}

static uint8_t functionGoto(int token) {
    insertGotoLabel();
    JP(0);

    return VALID;
}

void insertGotoLabel(void) {
    // Add the label to the stack, and skip the line
    label_t *gotoCurr = &ice.GotoStack[ice.curGoto++];
    uint8_t a = 0;
    int token;
    
    while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tColon && a < 20) {
        gotoCurr->name[a++] = token;
    }
    gotoCurr->name[a] = 0;
    gotoCurr->addr = ice.programPtr;
    gotoCurr->offset = _tell(ice.inPrgm);
    ResetAllRegs();
    
#ifdef CALCULATOR
    if (ice.debug) {
        gotoCurr->debugJumpDataPtr = ti_GetDataPtr(ice.dbgPrgm);
    }
#endif
}

static uint8_t functionPause(int token) {
    if (CheckEOL()) {
        MaybeLDIYFlags();
        CALL(_GetCSC);
        CP_A(9);
        JR_NZ(-8);
        reg.HLIsNumber = reg.HLIsVariable = false;
        reg.AIsNumber = true;
        reg.AIsVariable = false;
        reg.AValue = 9;
    } else {
        uint8_t res;

        SeekMinus1();
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        AnsToHL();

        CallRoutine(&ice.usedAlreadyPause, &ice.PauseAddr, (uint8_t*)PauseData, SIZEOF_PAUSE_DATA);
        reg.HLIsNumber = reg.DEIsNumber = true;
        reg.HLIsVariable = reg.DEIsVariable = false;
        reg.HLValue = reg.DEValue = -1;
        ResetBC();
    }

    return VALID;
}

static uint8_t functionInput(int token) {
    uint8_t res;

    expr.inFunction = true;
    if ((res = parseExpression(_getc())) != VALID) {
        return res;
    }

    if (ice.tempToken == tComma && expr.outputIsString) {
        expr.inFunction = true;
        if ((res = parseExpression(_getc())) != VALID) {
            return res;
        }
        *(ice.programPtr - 3) = 0x3E;
        *(ice.programPtr - 2) = *(ice.programPtr - 1);
        ice.programPtr--;
    } else {
        *(ice.programPtr - 3) = 0x3E;
        *(ice.programPtr - 2) = *(ice.programPtr - 1);
        ice.programPtr--;

        // FF0000 reads all zeroes, and that's important
        LD_HL_IMM(0xFF0000, TYPE_NUMBER);
    }

    if (ice.tempToken != tEnter || !expr.outputIsVariable) {
        return E_SYNTAX;
    }

    MaybeLDIYFlags();

    // Copy the Input routine to the data section
    if (!ice.usedAlreadyInput) {
        ice.programDataPtr -= SIZEOF_INPUT_DATA;
        ice.InputAddr = (uintptr_t)ice.programDataPtr;
        memcpy(ice.programDataPtr, (uint8_t*)InputData, SIZEOF_INPUT_DATA);
        ice.usedAlreadyInput = true;
    }

    // Set which var we need to store to
    ProgramPtrToOffsetStack();
    LD_ADDR_A(ice.InputAddr + SIZEOF_INPUT_DATA - 5);

    // Call the right routine
    ProgramPtrToOffsetStack();
    CALL(ice.InputAddr);
    ResetAllRegs();

    return VALID;
}

static uint8_t functionBB(int token) {
    // Asm(
    if ((uint8_t)(token = _getc()) == tAsm) {
        while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tColon && (uint8_t)token != tRParen) {
            uint8_t tok1, tok2;

            // Get hexadecimals
            if ((tok1 = IsHexadecimal(token)) == 16 || (tok2 = IsHexadecimal(_getc())) == 16) {
                return E_INVALID_HEX;
            }

            OutputWriteByte((tok1 << 4) + tok2);
        }
        if ((uint8_t)token == tRParen) {
            if (!CheckEOL()) {
                return E_SYNTAX;
            }
        }

        ResetAllRegs();

        return VALID;
    }

    // AsmComp(
    else if ((uint8_t)token == tAsmComp) {
        uint8_t res = VALID;
        uint24_t currentLine = ice.currentLine;
        ti_var_t tempProg = ice.inPrgm;
        prog_t *outputPrgm;
        
        outputPrgm = GetProgramName();
        if ((res = outputPrgm->errorCode) != VALID) {
            free(outputPrgm);
            return res;
        }

#ifndef CALCULATOR
        char *inName = str_dupcat(outputPrgm->prog, ".8xp");
        free(outputPrgm);
        if ((ice.inPrgm = _open(inName))) {
            int tempProgSize = ice.programLength;
            
            fseek(ice.inPrgm, 0, SEEK_END);
            ice.programLength = ftell(ice.inPrgm);
            _rewind(ice.inPrgm);
            fprintf(stdout, "Compiling subprogram %s\n", inName);
            free(inName);
            fprintf(stdout, "Program size: %u\n", ice.programLength);

            // Compile it, and close
            ice.currentLine = 0;
            if ((res = parseProgramUntilEnd()) != VALID) {
                return res;
            }
            fprintf(stdout, "Return from subprogram...\n");
            fclose(ice.inPrgm);
            ice.currentLine = currentLine;
            ice.programLength = tempProgSize;
        } else {
            free(inName);
            res = E_PROG_NOT_FOUND;
        }
#else
        if ((ice.inPrgm = _open(outputPrgm->prog))) {
            char buf[35];

            displayLoadingBarFrame();
            sprintf(buf, "Compiling subprogram %s...", outputPrgm->prog);
            displayMessageLineScroll(buf);
            strcpy(ice.currProgName[ice.inPrgm], outputPrgm->prog);
            free(outputPrgm);

            // Compile it, and close
            ice.currentLine = 0;
            if ((res = parseProgramUntilEnd()) != VALID) {
                return res;
            }
            ti_Close(ice.inPrgm);

            displayLoadingBarFrame();
            displayMessageLineScroll("Return from subprogram...");
            ice.currentLine = currentLine;
        } else {
            free(outputPrgm);
            res = E_PROG_NOT_FOUND;
        }
#endif
        ice.inPrgm = tempProg;

        return res;
    } else {
        SeekMinus1();
        
        return parseExpression(t2ByteTok);
    }
}

static uint8_t tokenWrongPlace(int token) {
    return E_WRONG_PLACE;
}

static uint8_t tokenUnimplemented(int token) {
    return E_UNIMPLEMENTED;
}

void optimizeZeroCarryFlagOutput(void) {
    if (!expr.AnsSetZeroFlag && !expr.AnsSetCarryFlag && !expr.AnsSetZeroFlagReversed && !expr.AnsSetCarryFlagReversed) {
        if (expr.outputRegister == REGISTER_HL) {
            ADD_HL_DE();
            OR_A_SBC_HL_DE();
            expr.AnsSetZeroFlag = true;
        } else if (expr.outputRegister == REGISTER_DE) {
            OutputWrite3Bytes(OP_SCF, 0xED, 0x62);
            ADD_HL_DE();
            expr.AnsSetCarryFlagReversed = true;
        } else {
            OR_A_A();
        }
    } else {
        ice.programPtr -= expr.ZeroCarryFlagRemoveAmountOfBytes;
    }
    return;
}

void skipLine(void) {
    while (!CheckEOL());
}

static uint8_t (*functions[256])(int) = {
    tokenUnimplemented, //0
    tokenUnimplemented, //1
    tokenUnimplemented, //2
    tokenUnimplemented, //3
    tokenWrongPlace,    //4
    tokenUnimplemented, //5
    tokenUnimplemented, //6
    tokenUnimplemented, //7
    parseExpression,    //8
    parseExpression,    //9
    tokenUnimplemented, //10
    parseExpression,    //11
    tokenUnimplemented, //12
    tokenUnimplemented, //13
    tokenUnimplemented, //14
    tokenUnimplemented, //15
    parseExpression,    //16
    parseExpression,    //17
    tokenUnimplemented, //18
    tokenUnimplemented, //19
    tokenUnimplemented, //20
    tokenUnimplemented, //21
    tokenUnimplemented, //22
    tokenUnimplemented, //23
    tokenUnimplemented, //24
    parseExpression,    //25
    parseExpression,    //26
    tokenUnimplemented, //27
    tokenUnimplemented, //28
    tokenUnimplemented, //29
    tokenUnimplemented, //30
    tokenUnimplemented, //31
    tokenUnimplemented, //32
    parseExpression,    //33
    tokenUnimplemented, //34
    tokenUnimplemented, //35
    tokenUnimplemented, //36
    tokenUnimplemented, //37
    tokenUnimplemented, //38
    tokenUnimplemented, //39
    tokenUnimplemented, //40
    tokenUnimplemented, //41
    parseExpression,    //42
    tokenUnimplemented, //43
    functionI,          //44
    tokenUnimplemented, //45
    tokenUnimplemented, //46
    tokenUnimplemented, //47
    parseExpression,    //48
    parseExpression,    //49
    parseExpression,    //50
    parseExpression,    //51
    parseExpression,    //52
    parseExpression,    //53
    parseExpression,    //54
    parseExpression,    //55
    parseExpression,    //56
    parseExpression,    //57
    tokenUnimplemented, //58
    parseExpression,    //59
    tokenWrongPlace,    //60
    tokenWrongPlace,    //61
    dummyReturn,        //62
    dummyReturn,        //63
    tokenWrongPlace,    //64
    parseExpression,    //65
    parseExpression,    //66
    parseExpression,    //67
    parseExpression,    //68
    parseExpression,    //69
    parseExpression,    //70
    parseExpression,    //71
    parseExpression,    //72
    parseExpression,    //73
    parseExpression,    //74
    parseExpression,    //75
    parseExpression,    //76
    parseExpression,    //77
    parseExpression,    //78
    parseExpression,    //79
    parseExpression,    //80
    parseExpression,    //81
    parseExpression,    //82
    parseExpression,    //83
    parseExpression,    //84
    parseExpression,    //85
    parseExpression,    //86
    parseExpression,    //87
    parseExpression,    //88
    parseExpression,    //89
    parseExpression,    //90
    parseExpression,    //91
    tokenUnimplemented, //92
    parseExpression,    //93
    tokenUnimplemented, //94
    functionPrgm,       //95
    tokenUnimplemented, //96
    tokenUnimplemented, //97
    functionCustom,     //98
    tokenUnimplemented, //99
    tokenUnimplemented, //100
    tokenUnimplemented, //101
    tokenUnimplemented, //102
    tokenUnimplemented, //103
    tokenUnimplemented, //104
    tokenUnimplemented, //105
    tokenWrongPlace,    //106
    tokenWrongPlace,    //107
    tokenWrongPlace,    //108
    tokenWrongPlace,    //109
    tokenWrongPlace,    //110
    tokenWrongPlace,    //111
    tokenWrongPlace,    //112
    tokenWrongPlace,    //113
    parseExpression,    //114
    tokenUnimplemented, //115
    tokenUnimplemented, //116
    tokenUnimplemented, //117
    tokenUnimplemented, //118
    tokenUnimplemented, //119
    tokenUnimplemented, //120
    tokenUnimplemented, //121
    tokenUnimplemented, //122
    tokenUnimplemented, //123
    tokenUnimplemented, //124
    tokenUnimplemented, //125
    tokenUnimplemented, //126
    tokenUnimplemented, //127
    tokenUnimplemented, //128
    tokenUnimplemented, //129
    parseExpression,    //130
    tokenWrongPlace,    //131
    tokenUnimplemented, //132
    tokenUnimplemented, //133
    tokenUnimplemented, //134
    tokenUnimplemented, //135
    tokenUnimplemented, //136
    tokenUnimplemented, //137
    tokenUnimplemented, //138
    tokenUnimplemented, //139
    tokenUnimplemented, //140
    tokenUnimplemented, //141
    tokenUnimplemented, //142
    tokenUnimplemented, //143
    tokenUnimplemented, //144
    tokenUnimplemented, //145
    tokenUnimplemented, //146
    tokenUnimplemented, //147
    tokenUnimplemented, //148
    tokenUnimplemented, //149
    tokenUnimplemented, //150
    tokenUnimplemented, //151
    tokenUnimplemented, //152
    tokenUnimplemented, //153
    tokenUnimplemented, //154
    tokenUnimplemented, //155
    tokenUnimplemented, //156
    tokenUnimplemented, //157
    tokenUnimplemented, //158
    tokenUnimplemented, //159
    tokenUnimplemented, //160
    tokenUnimplemented, //161
    tokenUnimplemented, //162
    tokenUnimplemented, //163
    tokenUnimplemented, //164
    tokenUnimplemented, //165
    tokenUnimplemented, //166
    tokenUnimplemented, //167
    tokenUnimplemented, //168
    tokenUnimplemented, //169
    parseExpression,    //170
    parseExpression,    //171
    parseExpression,    //172
    parseExpression,    //173
    parseExpression,    //174
    tokenUnimplemented, //175
    parseExpression,    //176
    tokenUnimplemented, //177
    tokenUnimplemented, //178
    parseExpression,    //179
    tokenUnimplemented, //180
    tokenUnimplemented, //181
    parseExpression,    //182
    tokenUnimplemented, //183
    parseExpression,    //184
    tokenUnimplemented, //185
    tokenUnimplemented, //186
    functionBB,         //187
    parseExpression,    //188
    tokenUnimplemented, //189
    tokenUnimplemented, //190
    tokenUnimplemented, //191
    parseExpression,    //192
    tokenUnimplemented, //193
    parseExpression,    //194
    tokenUnimplemented, //195
    parseExpression,    //196
    tokenUnimplemented, //197
    tokenUnimplemented, //198
    tokenUnimplemented, //199
    tokenUnimplemented, //200
    tokenUnimplemented, //201
    tokenUnimplemented, //202
    tokenUnimplemented, //203
    tokenUnimplemented, //204
    tokenUnimplemented, //205
    functionIf,         //206
    tokenUnimplemented, //207
    functionElse,       //208
    functionWhile,      //209
    functionRepeat,     //210
    functionFor,        //211
    functionEnd,        //212
    functionReturn,     //213
    functionLbl,        //214
    functionGoto,       //215
    functionPause,      //216
    tokenUnimplemented, //217
    tokenUnimplemented, //218
    tokenUnimplemented, //219
    functionInput,      //220
    tokenUnimplemented, //221
    functionDisp,       //222
    tokenUnimplemented, //223
    functionOutput,     //224
    functionClrHome,    //225
    tokenUnimplemented, //226
    tokenUnimplemented, //227
    tokenUnimplemented, //228
    tokenUnimplemented, //229
    tokenUnimplemented, //230
    tokenUnimplemented, //231
    tokenUnimplemented, //232
    tokenUnimplemented, //233
    tokenUnimplemented, //234
    tokenUnimplemented, //235
    tokenUnimplemented, //236
    tokenUnimplemented, //237
    tokenUnimplemented, //238
    parseExpression,    //239
    parseExpression,    //240
    tokenUnimplemented, //241
    tokenUnimplemented, //242
    tokenUnimplemented, //243
    tokenUnimplemented, //244
    tokenUnimplemented, //245
    tokenUnimplemented, //246
    tokenUnimplemented, //247
    tokenUnimplemented, //248
    tokenUnimplemented, //249
    tokenUnimplemented, //250
    tokenUnimplemented, //251
    tokenUnimplemented, //252
    tokenUnimplemented, //253
    tokenUnimplemented, //254
    tokenUnimplemented  //255
};
