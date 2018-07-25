#include "defines.h"
#include "parse.h"

#include "ast.h"
#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
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
element_t outputStack[400];
element_t stack[50];

uint8_t parseProgram(void) {
    uint8_t currentGoto, currentLbl, ret, *randAddr;
    
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
    if (ice.debug) {
        char buf[21];
        uint8_t curVar;
        
        sprintf(buf, "%.5sDBG", ice.outName);
        if (!(ice.dbgPrgm = ti_Open(buf, "w"))) {
            return E_NO_DBG_FILE;
        }
        
        // Write all variables to debug appvar
        sprintf(buf, "%s\x00", ice.currProgName[ice.inPrgm]);
        ti_Write(buf, strlen(buf) + 1, 1, ice.dbgPrgm);
        ti_PutC(prescan.amountOfVariablesUsed, ice.dbgPrgm);
        for (curVar = 0; curVar < prescan.amountOfVariablesUsed; curVar++) {
            sprintf(buf, "%s\x00", prescan.variables[curVar].name);
            ti_Write(buf, strlen(buf) + 1, 1, ice.dbgPrgm);
        }
    }
#endif

    if ((ret = parseProgramUntilEnd()) != VALID) {
        return ret;
    }
    
    if (!ice.lastTokenIsReturn) {
        RET();
    }
    
    // Find all the matching Goto's/Lbl's
    for (currentGoto = 0; currentGoto < ice.curGoto; currentGoto++) {
        label_t *curGoto = &ice.GotoStack[currentGoto];

        for (currentLbl = 0; currentLbl < ice.curLbl; currentLbl++) {
            label_t *curLbl = &ice.LblStack[currentLbl];

            if (!memcmp(curLbl->name, curGoto->name, 20)) {
                w24(curGoto->addr + 1, (uint24_t)curLbl->addr - (uint24_t)ice.programData + PRGM_START);
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
    
    // Do things based on the token
    while ((token = _getc()) != EOF) {
        uint8_t ret;
        
        ice.lastTokenIsReturn = false;
        ice.currentLine++;
        
        if ((ret = (*functions[token])(token)) != VALID) {
            return ret;
        }

#ifdef CALCULATOR
        displayLoadingBar();
#endif
    }
    
    return VALID;
}

uint8_t parseExpression(int token) {
    NODE *outputNode = NULL;
    NODE *stackNode = NULL;
    uint24_t stackElements = 0, outputElements = 0;
    uint24_t loopIndex, temp;
    uint8_t amountOfArgumentsStack[20];
    uint8_t index = 0, a, stackToOutputReturn, mask = TYPE_MASK_U24, tok, storeDepth = 0;
    uint8_t *amountOfArgumentsStackPtr = amountOfArgumentsStack, canUseMask = 2, prevTokenWasCFunction = 0;

    // Setup pointers
    element_t *outputPtr = outputStack;
    element_t *stackPtr  = stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    
    memset(&outputStack, 0, sizeof(outputStack));
    memset(&stack, 0, sizeof(stack));

    while (token != EOF && (tok = (uint8_t)token) != tEnter && tok != tColon) {
fetchNoNewToken:
        outputCurr = &outputPtr[outputElements];
        stackCurr  = &stackPtr[stackElements];

        // We can use the unsigned mask * only at the start of the line, or directly after an operator
        if (canUseMask) {
            canUseMask--;
        }

        // If there's a pointer directly after an -> operator, we have to ignore it
        if (storeDepth) {
            storeDepth--;
        }

        // If the previous token was a det( or a sum(, we need to store the next number in the stack entry too, to catch 'small arguments'
        if (prevTokenWasCFunction) {
            prevTokenWasCFunction--;
        }
        
        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;

            while ((uint8_t)(token = _getc()) >= t0 && (uint8_t)token <= t9) {
                output = output * 10 + token - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand.num = output;
            outputElements++;
            mask = TYPE_MASK_U24;

            if (prevTokenWasCFunction) {
                stackPtr[stackElements - 1].operand.func.function2 = output;
            }

            // Don't grab a new token
            continue;
        }

        // Process a hexadecimal number
        else if (tok == tee) {
            uint24_t output = 0;

            while ((tok = IsHexadecimal(token = _getc())) != 16) {
                output = (output << 4) + tok;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand.num = output;
            outputElements++;
            mask = TYPE_MASK_U24;

            // Don't grab a new token
            continue;
        }

        // Process a binary number
        else if (tok == tPi) {
            uint24_t output = 0;

            while ((tok = (token = _getc())) >= t0 && tok <= t1) {
                output = (output << 1) + tok - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand.num = output;
            outputElements++;
            mask = TYPE_MASK_U24;

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
                outputCurr->type = TYPE_NUMBER;
                outputCurr->operand.num = 0-output;
                outputElements++;
                mask = TYPE_MASK_U24;

                // Don't grab a new token
                continue;
            } else {
                // Pretend as if it's a -1*
                outputCurr->type = TYPE_NUMBER;
                outputCurr->operand.num = -1;
                outputElements++;
                SeekMinus1();
                tok = tMul;
                goto tokenIsOperator;
            }
        }

        // Process an OS list (number or list element)
        else if (tok == tVarLst) {
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand.num = prescan.OSLists[_getc()];
            outputElements++;
            mask = TYPE_MASK_U24;

            // Check if it's a list element
            if ((uint8_t)(token = _getc()) == tLParen) {
                // Trick ICE to think it's a {L1+...}
                *++amountOfArgumentsStackPtr = 1;
                stackCurr->type = TYPE_FUNCTION;
                stackCurr->mask = mask;

                // I have to create a non-existent token, because L1(...) the right parenthesis should pretend it's a },
                // but that is impossible if I just push { or (. Then when a ) appears and it hits the 0x0F, just replace it with a }
                stackCurr->operand.func.function = 0x0F;
                stackCurr->operand.func.function2 = storeDepth && 1;
                stackElements++;
                mask = TYPE_MASK_U24;
                canUseMask = 2;

                // :D
                token = tAdd;
            }

            // Don't grab next token
            continue;
        }

        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            outputCurr->type = TYPE_VARIABLE;
            outputCurr->operand.var = GetVariableOffset(tok);
            outputElements++;
            mask = TYPE_MASK_U24;
        }

        // Process a mask
        else if (tok == tMul && canUseMask) {
            uint8_t a = 0;

            while ((uint8_t)(token = _getc()) == tMul) {
                a++;
            }
            if (a > 2 || (uint8_t)token != tLBrace) {
                return E_SYNTAX;
            }
            mask = TYPE_MASK_U8 + a;

            // If the previous token was a ->, remind it, if not, this won't hurt
            storeDepth++;

            // Don't grab the { token
            continue;
        }

        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            // If the token is ->, move the entire stack to the output, instead of checking the precedence
            if (tok == tStore) {
                storeDepth = 2;
                // Move entire stack to output
                stackToOutputReturn = 1;
                goto stackToOutput;
            }
tokenIsOperator:

            // Move the stack to the output as long as it's not empty
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];

                // Move the last entry of the stack to the ouput if it's precedence is greater than the precedence of the current token
                if (stackPrev->type == TYPE_OPERATOR && operatorPrecedence[index - 1] <= operatorPrecedence2[getIndexOfOperator(stackPrev->operand.op) - 1]) {
                    memcpy(&outputPtr[outputElements], &stackPtr[stackElements-1], sizeof(element_t));
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }

stackToOutputReturn1:
            // Push the operator to the stack
            stackCurr = &stackPtr[stackElements++];
            stackCurr->type = TYPE_OPERATOR;
            stackCurr->operand.op = token;
            mask = TYPE_MASK_U24;
            canUseMask = 2;
        }

        // Gets the address of a variable
        else if (tok == tFromDeg) {
            outputCurr->type = TYPE_NUMBER;
            outputElements++;
            mask = TYPE_MASK_U24;
            tok = _getc();

            // Get the address of the variable
            if (tok >= tA && tok <= tTheta) {
                outputCurr->operand.num = IX_VARIABLES + (char)GetVariableOffset(tok);
            } else if (tok == tVarLst) {
                outputCurr->operand.num = prescan.OSLists[_getc()];
            } else if (tok == tVarStrng) {
                outputCurr->operand.num = prescan.OSStrings[_getc()];
            } else {
                return E_SYNTAX;
            }
        }

        // Pop a ) } ,
        else if (tok == tRParen || tok == tComma || tok == tRBrace) {
            uint8_t tempTok, index;

            // Move until stack is empty or a function is encountered
            while (stackElements) {
                stackPrev = &stackPtr[stackElements - 1];
                outputCurr = &outputPtr[outputElements];
                if (stackPrev->type != TYPE_FUNCTION) {
                    memcpy(outputCurr, stackPrev, sizeof(element_t));
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }

            // No matching left parenthesis
            if (!stackElements) {
                if (expr.inFunction) {
                    ice.tempToken = tok;
                    
                    goto stopParsing;
                }
                
                return E_EXTRA_RPAREN;
            }
            
            stackPrev = &stackPtr[stackElements - 1];
            tempTok = stackPrev->operand.num;

            // Closing tag should match it's open tag
            if ((tok == tRBrace && (tempTok != token - 1)) || (tok == tRParen && tempTok != 0x0F && tempTok == tLBrace)) {
                return E_SYNTAX;
            }

            index = GetIndexOfFunction(tempTok, stackPrev->operand.func.function2);

            // If it's a det, add an argument delimiter as well
            if (tok == tComma && index != 255 && implementedFunctions[index].pushBackwards) {
                outputCurr->type = TYPE_ARG_DELIMITER;
                outputElements++;
            }

            // If the right parenthesis belongs to a function, move the function as well
            if (tok != tComma) {
                memcpy(outputCurr, stackPrev, sizeof(element_t));
                outputCurr->operand.func.amountOfArgs = *amountOfArgumentsStackPtr--;
                if (stackPrev->operand.func.function == 0x0F) {
                    outputCurr->operand.func.function = tLBrace;
                }
                outputElements++;

                // If you moved the function or not, it should always pop the last stack element
                stackElements--;
            }

            // Increment the amount of arguments for that function
            else {
                (*amountOfArgumentsStackPtr)++;
                canUseMask = 2;
            }

            mask = TYPE_MASK_U24;
        }

        // getKey / getKey(
        else if (tok == tGetKey) {
            mask = TYPE_MASK_U24;
            if ((uint8_t)(token = _getc()) == tLParen) {
                *++amountOfArgumentsStackPtr = 1;
                stackCurr->type = TYPE_FUNCTION;
                stackCurr->operand.func.function = tGetKey;
                stackElements++;
                canUseMask = 2;
            } else {
                outputCurr->type = TYPE_FUNCTION;
                outputCurr->operand.func.function = tGetKey;
                outputElements++;
                
                continue;
            }
        }

        // Parse a string of tokens
        else if (tok == tAPost) {
            uint8_t *tempProgramPtr = ice.programPtr;
            uint24_t length;

            outputCurr->isString = true;
            outputCurr->type = TYPE_STRING;
            outputElements++;
            mask = TYPE_MASK_U24;

            while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tColon && (uint8_t)token != tStore && (uint8_t)token != tAPost) {
                OutputWriteByte(token);

                if (IsA2ByteTok(token)) {
                    OutputWriteByte(_getc());
                }
            }

            OutputWriteByte(0);

            length = ice.programPtr - tempProgramPtr;
            ice.programDataPtr -= length;
            memcpy(ice.programDataPtr, tempProgramPtr, length);
            ice.programPtr = tempProgramPtr;

            outputCurr->operand.num = (uint24_t)ice.programDataPtr;

            if ((uint8_t)token == tStore || (uint8_t)token == tEnter || (uint8_t)token == tColon) {
                continue;
            }
        }

        // Parse a string of characters
        else if (tok == tString) {
            uint24_t length;
            uint8_t *tempDataPtr = ice.programPtr, *a;
            uint8_t amountOfHexadecimals = 0;
            bool needWarning = true;

            outputCurr->isString = true;
            outputCurr->type = TYPE_STRING;
            outputElements++;
            mask = TYPE_MASK_U24;
            stackPrev = &stackPtr[stackElements - 1];

            token = grabString(&ice.programPtr, true);
            if (stackElements && (uint8_t)stackPrev->operand.num == tVarOut && stackPrev->operand.func.function2 == tDefineSprite) {
                needWarning = false;
            }
            
            for (a = tempDataPtr; a < ice.programPtr; a++) {
                if (IsHexadecimal(*a) == 16) {
                    goto noSquishing;
                }
                amountOfHexadecimals++;
            }
            if (!(amountOfHexadecimals & 1)) {
                uint8_t *prevDataPtr = tempDataPtr;
                uint8_t *prevDataPtr2 = tempDataPtr;

                while (prevDataPtr != ice.programPtr) {
                    uint8_t tok1 = IsHexadecimal(*prevDataPtr++);
                    uint8_t tok2 = IsHexadecimal(*prevDataPtr++);

                    *prevDataPtr2++ = (tok1 << 4) + tok2;
                }

                ice.programPtr = prevDataPtr2;

                if (needWarning) {
                    displayError(W_SQUISHED);
                }
            }

noSquishing:
            OutputWriteByte(0);
            
            length = ice.programPtr - tempDataPtr;
            ice.programDataPtr -= length;
            memcpy(ice.programDataPtr, tempDataPtr, length);
            ice.programPtr = tempDataPtr;
            
            outputCurr->operand.num = (uint24_t)ice.programDataPtr;

            if ((uint8_t)token == tStore || (uint8_t)token == tEnter || (uint8_t)token == tColon) {
                continue;
            }
        }

        // Parse an OS string
        else if (tok == tVarStrng) {
            outputCurr->isString = true;
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand.num = prescan.OSStrings[_getc()];
            outputElements++;
            mask = TYPE_MASK_U24;
        }

        // Parse a function
        else {
            uint8_t index, tok2 = 0;

            if (IsA2ByteTok(tok)) {
                tok2 = _getc();
            }
            
            if ((index = GetIndexOfFunction(tok, tok2)) != 255) {
                // LEFT and RIGHT should have a left paren associated with it
                if (tok == tExtTok && (tok2 == tLEFT || tok2 == tRIGHT)) {
                    if ((uint8_t)_getc() != tLParen) {
                        return E_SYNTAX;
                    }
                }
                
                if (implementedFunctions[index].amountOfArgs) {
                    // We always have at least 1 argument
                    *++amountOfArgumentsStackPtr = 1;
                    stackCurr->type = TYPE_FUNCTION;
                    stackCurr->mask = mask;
                    stackCurr->operand.num = tok + (((tok == tLBrace && storeDepth) + tok2) << 16);
                    stackElements++;
                    mask = TYPE_MASK_U24;
                    canUseMask = 2;

                    // Check if it's a function with pushed arguments
                    if (implementedFunctions[index].pushBackwards) {
                        outputCurr->type = TYPE_C_START;
                        outputElements++;
                        
                        tok2 = token = _getc();

                        if ((tok == tDet || tok == tSum) && (tok2 < t0 || tok2 > t9)) {
                            return E_SYNTAX;
                        }
                        prevTokenWasCFunction = 2;
                        tok = tok2;

                        goto fetchNoNewToken;
                    }
                } else {
                    outputCurr->type = TYPE_FUNCTION;
                    outputCurr->operand.num = (tok2 << 16) + tok;
                    outputElements++;
                    mask = TYPE_MASK_U24;
                }
                
                goto fetchNewToken;
            }

            // Oops, unknown token...
            return E_UNIMPLEMENTED;
        }

        // Yay, fetch the next token, it's great, it's true, I like it
fetchNewToken:
        token = _getc();
    }

    // If the expression quits normally, rather than an argument seperator
    ice.tempToken = tEnter;

stopParsing:
    // Move entire stack to output
    stackToOutputReturn = 2;
    goto stackToOutput;
stackToOutputReturn2:

    // Remove stupid things like 2+5, and not(1, max(2,3
    for (loopIndex = 1; loopIndex < outputElements; loopIndex++) {
        outputPrevPrev = &outputPtr[loopIndex - 2];
        outputPrev = &outputPtr[loopIndex - 1];
        outputCurr = &outputPtr[loopIndex];
        index = outputCurr->operand.func.amountOfArgs;

        // Check if the types are number | number | operator (not both OS strings though)
        if (loopIndex > 1 && outputPrevPrev->type == TYPE_NUMBER && outputPrev->type == TYPE_NUMBER &&
               !(outputPrevPrev->isString && outputPrev->isString) && 
               outputCurr->type == TYPE_OPERATOR && outputCurr->operand.op != tStore) {
            // If yes, execute the operator, and store it in the first entry, and remove the other 2
            outputPrevPrev->operand.num = executeOperator(outputPrevPrev->operand.num, outputPrev->operand.num, outputCurr->operand.op);
            memcpy(outputPrev, &outputPtr[loopIndex + 1], (outputElements - 1) * sizeof(element_t));
            outputElements -= 2;
            loopIndex -= 2;
            continue;
        }

        // Check if the types are number | number | ... | function (specific function or pointer)
        if (loopIndex >= index && outputCurr->type == TYPE_FUNCTION) {
            uint8_t a, index2, function = outputCurr->operand.func.function, function2 = outputCurr->operand.func.function2;
            
            if ((index2 = GetIndexOfFunction(function, function2)) != 255 && implementedFunctions[index2].numbersArgs) {
                uint24_t outputPrevOperand = outputPrev->operand.num, outputPrevPrevOperand = outputPrevPrev->operand.num;

                for (a = 1; a <= index; a++) {
                    if (outputPtr[loopIndex-a].type != TYPE_NUMBER) {
                        goto DontDeleteFunction;
                    }
                }

                switch (function) {
                    case tNot:
                        temp = !outputPrevOperand;
                        break;
                    case tMin:
                        temp = (outputPrevOperand < outputPrevPrevOperand) ? outputPrevOperand : outputPrevPrevOperand;
                        break;
                    case tMax:
                        temp = (outputPrevOperand > outputPrevPrevOperand) ? outputPrevOperand : outputPrevPrevOperand;
                        break;
                    case tMean:
                        // I can't simply add, and divide by 2, because then it *might* overflow in case that A + B > 0xFFFFFF
                        temp = ((long)outputPrevOperand + (long)outputPrevPrevOperand) / 2;
                        break;
                    case tSqrt:
                        temp = sqrt(outputPrevOperand);
                        break;
                    case tExtTok:
                        if (function2 == tRemainder) {
                            temp = outputPrevOperand % outputPrevPrevOperand;
                        } else if (function2 == tLEFT) {
                            temp = outputPrevPrevOperand << outputPrevOperand;
                        } else if (function2 == tRIGHT) {
                            temp = outputPrevPrevOperand >> outputPrevOperand;
                        } else {
                            return E_ICE_ERROR;
                        }
                        break;
                    case tSin:
                        temp = 255*sin((double)outputPrevOperand * (2 * M_PI / 256));
                        break;
                    case tCos:
                        temp = 255*cos((double)outputPrevOperand * (2 * M_PI / 256));
                        break;
                    default:
                        return E_ICE_ERROR;
                }

                // And remove everything
                outputPtr[loopIndex - index].operand.num = temp;
                memmove(&outputPtr[loopIndex - index + 1], &outputPtr[loopIndex + 1], (outputElements - 1) * sizeof(element_t));
                outputElements -= index;
                loopIndex -= index - 1;
            }
        }
DontDeleteFunction:;
    }

    // Check if the expression is valid
    if (!outputElements) {
        return E_SYNTAX;
    }

    return parsePostFixFromIndexToIndex(0, outputElements - 1);

    // Duplicated function opt
stackToOutput:
    // Move entire stack to output
    while (stackElements) {
        outputCurr = &outputPtr[outputElements++];
        stackPrev = &stackPtr[--stackElements];

        temp = stackPrev->operand.num;
        if ((uint8_t)temp == 0x0F) {
            // :D
            temp = (temp & 0xFF0000) + tLBrace;
        }

        // If it's a function, add the amount of arguments as well
        if (stackPrev->type == TYPE_FUNCTION) {
            temp += (*amountOfArgumentsStackPtr--) << 8;
        }

        outputCurr->isString = stackPrev->isString;
        outputCurr->type = stackPrev->type;
        outputCurr->mask = stackPrev->mask;
        outputCurr->operand.num = temp;
    }

    // Select correct return location
    if (stackToOutputReturn == 2) {
        goto stackToOutputReturn2;
    }

    goto stackToOutputReturn1;
}

uint8_t parsePostFixFromIndexToIndex(uint24_t startIndex, uint24_t endIndex) {
    element_t *outputCurr;
    element_t *outputPtr = (element_t*)outputStack;
    uint8_t outputType, temp, AnsDepth = 0;
    uint24_t outputOperand, loopIndex, tempIndex = 0, amountOfStackElements;

    // Set some variables
    outputCurr = &outputPtr[startIndex];
    outputType = outputCurr->type;
    outputOperand = outputCurr->operand.num;
    ice.stackStart = (uint24_t*)(ice.stackDepth * STACK_SIZE + ice.stack);
    setStackValues(ice.stackStart, ice.stackStart);
    reg.allowedToOptimize = true;

    // Clean the expr struct
    memset(&expr, 0, sizeof expr);

    // Get all the indexes of the expression
    temp = 0;
    amountOfStackElements = 0;
    
    for (loopIndex = startIndex; loopIndex <= endIndex; loopIndex++) {
        uint8_t index;
        
        outputCurr = &outputPtr[loopIndex];
        index = GetIndexOfFunction(outputCurr->operand.num, outputCurr->operand.func.function2);

        // If it's the start of a det( or sum(, increment the amount of nested det(/sum(
        if (outputCurr->type == TYPE_C_START) {
            temp++;
        }
        // If it's a det( or sum(, decrement the amount of nested dets
        if (outputCurr->type == TYPE_FUNCTION && index != 255 && implementedFunctions[index].pushBackwards) {
            temp--;
            amountOfStackElements++;
        }

        // If not in a nested det( or sum(, push the index
        if (!temp) {
            push(loopIndex);
            amountOfStackElements++;
        }
    }

    // Empty argument
    if (!amountOfStackElements) {
        return E_SYNTAX;
    }

    // It's a single entry
    if (amountOfStackElements == 1) {
        // Expression is a string
        if (outputCurr->isString) {
            expr.outputIsString = true;
            LD_HL_STRING(outputOperand, outputType);
        }
        
        // Expression is only a single number
         else if (outputType == TYPE_NUMBER) {
            // This boolean is set, because loops may be optimized when the condition is a number
            expr.outputIsNumber = true;
            expr.outputNumber = outputOperand;
            LD_HL_IMM(outputOperand);
        }

        // Expression is only a variable
        else if (outputType == TYPE_VARIABLE) {
            expr.outputIsVariable = true;
            OutputWriteWord(0x27DD);
            OutputWriteByte(outputOperand);
            reg.HLIsNumber = false;
            reg.HLIsVariable = true;
            reg.HLVariable = outputOperand;
        }

        // Expression is an empty function or operator, i.e. not(, +
        else {
            return E_SYNTAX;
        }

        return VALID;
    }

    // 3 or more entries, full expression
    do {
        element_t *outputPrevPrevPrev;

        outputCurr = &outputPtr[loopIndex = getNextIndex()];
        outputPrevPrevPrev = &outputPtr[getIndexOffset(-4)];
        outputType = outputCurr->type;
        
        // Set some vars
        expr.outputReturnRegister = REGISTER_HL;
        expr.outputIsString = false;

        if (outputType == TYPE_OPERATOR) {
            element_t *outputPrev, *outputPrevPrev, *outputNext, *outputNextNext;
            bool canOptimizeConcatenateStrings;

            // Wait, invalid operator?!
            if (loopIndex < startIndex + 2) {
                return E_SYNTAX;
            }

            if (AnsDepth > 3 && (uint8_t)outputCurr->operand.num != tStore) {
                // We need to push HL since it isn't used in the next operator/function
                outputPtr[tempIndex].type = TYPE_CHAIN_PUSH;
                PushHLDE();
                expr.outputRegister = REGISTER_HL;
            }

            // Get the previous entries, -2 is the previous one, -3 is the one before etc
            outputPrev     = &outputPtr[getIndexOffset(-2)];
            outputPrevPrev = &outputPtr[getIndexOffset(-3)];
            outputNext     = &outputPtr[getIndexOffset(0)];
            outputNextNext = &outputPtr[getIndexOffset(1)];
            
            // Check if we can optimize StrX + "..." -> StrX
            canOptimizeConcatenateStrings = (
                (uint8_t)(outputCurr->operand.num) == tAdd &&
                outputPrevPrev->isString && outputPrevPrev->type == TYPE_NUMBER &&
                outputNext->isString && outputNext->type == TYPE_NUMBER &&
                outputNext->operand.num == outputPrevPrev->operand.num &&
                outputNextNext->type == TYPE_OPERATOR &&
                (uint8_t)(outputNextNext->operand.num) == tStore
            );

            // Parse the operator with the 2 latest operands of the stack!
            if ((temp = parseOperator(outputPrevPrevPrev, outputPrevPrev, outputPrev, outputCurr, canOptimizeConcatenateStrings)) != VALID) {
                return temp;
            }

            // Remove the index of the first and the second argument, the index of the operator will be the chain
            removeIndexFromStack(getCurrentIndex() - 2);
            removeIndexFromStack(getCurrentIndex() - 2);
            AnsDepth = 0;

            // Eventually remove the ->StrX too
            if (canOptimizeConcatenateStrings) {
                loopIndex = getIndexOffset(1);
                removeIndexFromStack(getCurrentIndex());
                removeIndexFromStack(getCurrentIndex() + 1);

                outputCurr->isString = true;
                outputCurr->type = TYPE_NUMBER;
                outputCurr->operand.num = outputPrevPrev->operand.num;
                expr.outputIsString = true;
            } else {
                // Check if it was a command with 2 strings, then the output is a string, not Ans
                if ((uint8_t)outputCurr->operand.num == tAdd && outputPrevPrev->isString && outputPrev->isString) {
                    outputCurr->isString = true;
                    outputCurr->type = TYPE_STRING;
                    if (outputPrevPrev->operand.num == prescan.tempStrings[TempString2] || outputPrev->operand.num == prescan.tempStrings[TempString1]) {
                        outputCurr->operand.num = prescan.tempStrings[TempString2];
                    } else {
                        outputCurr->operand.num = prescan.tempStrings[TempString1];
                    }
                    expr.outputIsString = true;
                } else {
                    AnsDepth = 1;
                    outputCurr->type = TYPE_CHAIN_ANS;
                }
                tempIndex = loopIndex;
            }
        }

        else if (outputType == TYPE_FUNCTION) {
            // Use this to cleanup the function after parsing
            uint8_t amountOfArguments = outputCurr->operand.func.amountOfArgs;
            uint8_t function2 = outputCurr->operand.func.function2;
            
            // Only execute when it's not a pointer directly after a ->
            if (outputCurr->operand.num != 0x010108) {
                uint8_t index = GetIndexOfFunction(outputCurr->operand.num, function2);
                
                // Check if we need to push Ans
                if (AnsDepth > 1 + amountOfArguments || (AnsDepth && implementedFunctions[index].pushBackwards)) {
                    // We need to push HL since it isn't used in the next operator/function
                    outputPtr[tempIndex].type = TYPE_CHAIN_PUSH;
                    PushHLDE();
                    expr.outputRegister = REGISTER_HL;
                }
                
                if (amountOfArguments != implementedFunctions[index].amountOfArgs && implementedFunctions[index].amountOfArgs != 255) {
                    return E_ARGUMENTS;
                }

                if ((temp = parseFunction(loopIndex)) != VALID) {
                    return temp;
                }

                // Cleanup, if it's not a det(
                if (index == 255 || !implementedFunctions[index].pushBackwards) {
                    for (temp = 0; temp < amountOfArguments; temp++) {
                        removeIndexFromStack(getCurrentIndex() - 2);
                    }
                }

                // I don't care that this will be ignored when it's a pointer, because I know there is a -> directly after
                // If it's a sub(, the output should be a string, not Ans
                if ((uint8_t)outputCurr->operand.num == t2ByteTok && function2 == tSubStrng) {
                    outputCurr->isString = true;
                    outputCurr->type = TYPE_STRING;
                    if (outputPrevPrevPrev->operand.num == prescan.tempStrings[TempString1]) {
                        outputCurr->operand.num = prescan.tempStrings[TempString2];
                    } else {
                        outputCurr->operand.num = prescan.tempStrings[TempString1];
                    }
                    expr.outputIsString = true;
                    AnsDepth = 0;
                }

                // Check chain push/ans
                else {
                    AnsDepth = 1;
                    tempIndex = loopIndex;
                    outputCurr->type = TYPE_CHAIN_ANS;
                }
            }
        }

        if (AnsDepth) {
            AnsDepth++;
        }
    } while (loopIndex != endIndex);

    return VALID;
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
        uint8_t *IfStartAddr, res;
        uint24_t tempDataOffsetElements;

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
            uint8_t tempGotoElements2 = ice.curGoto;
            uint8_t tempLblElements2 = ice.curLbl;
            uint24_t tempDataOffsetElements2;

            // Backup stuff
            ResetAllRegs();
            IfElseAddr = ice.programPtr;
            tempDataOffsetElements2 = ice.dataOffsetElements;

            JP(0);
            if ((res = parseProgramUntilEnd()) != E_END && res != VALID) {
                return res;
            }

            shortElseCode = JumpForward(IfElseAddr, ice.programPtr, tempDataOffsetElements2, tempGotoElements2, tempLblElements2);
            JumpForward(IfStartAddr, IfElseAddr + (shortElseCode ? 2 : 4), tempDataOffsetElements, tempGotoElements, tempLblElements);
        }

        // Check if we quit the program with an 'End' or at the end of the input program
        else if (res == E_END || res == VALID) {
            JumpForward(IfStartAddr, ice.programPtr, tempDataOffsetElements, tempGotoElements, tempLblElements);
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
    if (endAddr - startAddr <= 0x80) {
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

    // Basically the same as "Repeat", but jump to condition checking first
    JP(0);
    if ((res = functionRepeat(token)) != VALID) {
        return res;
    }
    WhileJumpForwardSmall = JumpForward(WhileStartAddr, WhileRepeatCondStart, tempDataOffsetElements, tempGotoElements, tempLblElements);
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

    // Parse the code inside the loop
    if ((res = parseProgramUntilEnd()) != E_END && res != VALID) {
        return res;
    }

    ResetAllRegs();

    // Remind where the "End" is
    RepeatProgEnd = _tell(ice.inPrgm);
    if (token == tWhile) {
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
    
    if (ice.tempToken != tRParen && ice.tempToken != tEnter) {
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
                LD_DE_IMM(stepNumber);
                ADD_HL_DE();
            }
        } else {
            w24(stepExpression + 1, ice.programPtr + PRGM_START - ice.programData + 1);
            ice.ForLoopSMCStack[ice.ForLoopSMCElements++] = (uint24_t*)(endPointExpressionValue + 1);

            LD_DE_IMM(0);
            ADD_HL_DE();
        }
        LD_IX_OFF_IND_HL(variable);
    }

    smallCode = JumpForward(jumpToCond, ice.programPtr, tempDataOffsetElements, tempGotoElements, tempLblElements);
    ResetAllRegs();

    // If both the step and the end point are a number, the variable is already in HL
    if (!(endPointIsNumber && stepIsNumber)) {
        LD_HL_IND_IX_OFF(variable);
    }

    if (endPointIsNumber) {
        if (stepNumber < 0x800000) {
            LD_DE_IMM(endPointNumber + 1);
        } else {
            LD_DE_IMM(endPointNumber);
            reversedCond = true;
        }
        OR_A_A();
    } else {
        w24(endPointExpressionValue + 1, ice.programPtr + PRGM_START - ice.programData + 1);
        ice.ForLoopSMCStack[ice.ForLoopSMCElements++] = (uint24_t*)(endPointExpressionValue + 1);

        LD_DE_IMM(0);
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

    return VALID;
}

static uint8_t functionPrgm(int token) {
    uint24_t length;
    uint8_t a = 0;
    uint8_t *tempProgramPtr;

    MaybeLDIYFlags();
    tempProgramPtr = ice.programPtr;

    OutputWriteByte(TI_PRGM_TYPE);

    // Fetch the name
    while ((token = _getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tColon && ++a < 9) {
        OutputWriteByte(token);
    }
    OutputWriteByte(0);

    // Check if valid program name
    if (!a || a == 9) {
        return E_INVALID_PROG;
    }

    length = ice.programPtr - tempProgramPtr;
    ice.programDataPtr -= length;
    memcpy(ice.programDataPtr, tempProgramPtr, length);
    ice.programPtr = tempProgramPtr;

    ProgramPtrToOffsetStack();
    LD_HL_IMM((uint24_t)ice.programDataPtr);

    // Insert the routine to run it
    CALL(_Mov9ToOP1);
    LD_HL_IMM(tempProgramPtr - ice.programData + PRGM_START + 28);
    memcpy(ice.programPtr, PrgmData, 20);
    ice.programPtr += 20;
    ResetAllRegs();

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
        LD_HL_IMM(0xFF0000);
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
    tokenUnimplemented, //192
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
