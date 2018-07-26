#include "defines.h"
#include "prescan.h"

#include "ast.h"
#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "functions.h"
#include "routines.h"
#include "errors.h"

#ifndef CALCULATOR
extern const uint8_t CheaderData[];
extern const uint8_t FileiocheaderData[];

extern char *str_dupcat(const char *s, const char *c);
#endif

extern const uint8_t implementedFunctions[AMOUNT_OF_FUNCTIONS][4];
prescan_t prescan;
const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};    // Thanks Cesium :D

void preScanProgram(void) {
    bool inString = false, afterNewLine = true;
    int token;
    
    _rewind(ice.inPrgm);

    // Scan the entire program
    while ((token = _getc()) != EOF) {
        uint8_t tok = token, tok2 = 0;
        
        if (IsA2ByteTok(tok)) {
            tok2 = _getc();
        }

        if (afterNewLine) {
            afterNewLine = false;
            if (tok == tii) {
                skipLine();
                afterNewLine = true;
            } else if (tok == tLbl) {
                prescan.amountOfLbls++;
                skipLine();
                afterNewLine = true;
            } else if (tok == tGoto || (tok == tVarOut && tok2 == tCall)) {
                prescan.amountOfGotos++;
                skipLine();
                afterNewLine = true;
            }
        }

        if (tok == tString) {
            prescan.usedTempStrings = true;
            inString = !inString;
        } else if (tok == tStore) {
            inString = false;
        } else {
            if (tok == tEnter || (tok == tColon && !inString)) {
                inString = false;
                afterNewLine = true;
            } else if (!inString) {
                if (tok >= tA && tok <= tTheta) {
                    GetVariableOffset(tok);
                } else if (tok == tRand) {
                    prescan.amountOfRandRoutines++;
                    prescan.modifiedIY = true;
                } else if (tok == tSqrt) {
                    prescan.amountOfSqrtRoutines++;
                    prescan.modifiedIY = true;
                } else if (tok == tMean) {
                    prescan.amountOfMeanRoutines++;
                } else if (tok == tInput) {
                    prescan.amountOfInputRoutines++;
                } else if (tok == tPause) {
                    prescan.amountOfPauseRoutines++;
                } else if (tok == tVarLst) {
                    if (!prescan.OSLists[tok2]) {
                        prescan.OSLists[tok2] = pixelShadow + 2000 * (prescan.amountOfOSVarsUsed++);
                    }
                } else if (tok == tVarStrng) {
                    prescan.usedTempStrings = true;
                    if (!prescan.OSStrings[tok2]) {
                        prescan.OSStrings[tok2] = pixelShadow + 2000 * (prescan.amountOfOSVarsUsed++);
                    }
                } else if (tok == t2ByteTok) {
                    // AsmComp(
                    if (tok2 == tAsmComp) {
                        prog_t *newProg = GetProgramName();
                        if (newProg->errorCode == VALID) {
                            ti_var_t tempProg = ice.inPrgm;
#ifdef CALCULATOR
                            if ((ice.inPrgm = _open(newProg->prog))) {
#else
                            char *inName = str_dupcat(newProg->prog, ".8xp");
			    if ((ice.inPrgm = _open(inName))) {
#endif
                                preScanProgram();
                                _close(ice.inPrgm);
                            }
#ifndef CALCULATOR
                            free(inName);
#endif
                            ice.inPrgm = tempProg;
                        }
                        free(newProg);
                        afterNewLine = true;
                    } else if (tok2 == tRandInt) {
                        prescan.amountOfRandRoutines++;
                        prescan.modifiedIY = true;
                    }
                } else if (tok == tExtTok) {
                    if (tok2 == tStartTmr) {
                        prescan.amountOfTimerRoutines++;
                    }
                } else if (tok == tDet || tok == tSum) {
                    uint8_t tok1 = _getc();
                    tok2 = _getc();

                    prescan.modifiedIY = true;

                    // Invalid det( command
                    if (tok1 < t0 || tok1 > t9) {
                        break;
                    }

                    // Get the det( command
                    if (tok2 < t0 || tok2 > t9) {
                        token = tok1 - t0;
                        if (tok2 == tEnter || tok2 == tColon) {
                            afterNewLine = true;
                        }
                    } else {
                        token = (tok1 - t0) * 10 + (tok2 - t0);
                    }

                    if (tok == tDet) {
                        prescan.hasGraphxFunctions = true;
                        if (!token) {
                            prescan.hasGraphxStart = true;
                        }
                        if (token == 1) {
                            prescan.hasGraphxEnd = true;
                        }
                        if (!prescan.GraphxRoutinesStack[token]) {
                            prescan.GraphxRoutinesStack[token] = 1;
                        }
                    } else {
                        prescan.hasFileiocFunctions = true;
                        if (!token) {
                            prescan.hasFileiocStart = true;
                        }
                        if (!prescan.FileiocRoutinesStack[token]) {
                            prescan.FileiocRoutinesStack[token] = 1;
                        }
                    }
                }
            }
        }
    }

    _rewind(ice.inPrgm);
}

uint8_t getNameIconDescription(void) {
    uint8_t res;
    prog_t *outputPrgm;
    
    if (_getc() != 0x2C) {
        return E_NOT_ICE_PROG;
    }
    
    outputPrgm = GetProgramName();
    if ((res = outputPrgm->errorCode) != VALID) {
        free(outputPrgm);
        return res;
    }
    strcpy(ice.outName, outputPrgm->prog);
    free(outputPrgm);
    
    // Has icon
    if ((uint8_t)_getc() == tii && (uint8_t)_getc() == tString) {
        uint8_t b = 0;

        *ice.programPtr = OP_JP;
        w24(ice.programPtr + 4, 0x101001);
        ice.programPtr += 7;

        // Get hexadecimal
        do {
            uint8_t temp;
            
            if ((temp = IsHexadecimal(_getc())) == 16) {
                return E_INVALID_HEX;
            }
            OutputWriteByte(colorTable[temp]);
        } while (++b);

        if ((uint8_t)_getc() != tString || (uint8_t)_getc() != tEnter) {
            return E_SYNTAX;
        }

        // Check description
        if ((uint8_t)_getc() == tii) {
            grabString(&ice.programPtr, false);
        }
        OutputWriteByte(0);

        // Write the right jp offset
        w24(ice.programData + 1, ice.programPtr - ice.programData + PRGM_START);
    }
    
    _rewind(ice.inPrgm);
    
    return VALID;
}

uint8_t parsePrescan(void) {
    // Insert C functions
    if (prescan.hasGraphxFunctions) {
        uint8_t a;

        memcpy(ice.programPtr, CheaderData, SIZEOF_CHEADER);
        ice.programPtr += SIZEOF_CHEADER;
        for (a = 0; a < AMOUNT_OF_GRAPHX_FUNCTIONS; a++) {
            if (prescan.GraphxRoutinesStack[a]) {
                prescan.GraphxRoutinesStack[a] = (uint24_t)ice.programPtr;
                JP(a * 3);
            }
        }
    } else if (prescan.hasFileiocFunctions) {
        memcpy(ice.programPtr, CheaderData, SIZEOF_CHEADER - 9);
        ice.programPtr += SIZEOF_CHEADER - 9;
    }

    if (prescan.hasFileiocFunctions) {
        uint8_t a;

        memcpy(ice.programPtr, FileiocheaderData, 10);
        ice.programPtr += 10;
        for (a = 0; a < AMOUNT_OF_FILEIOC_FUNCTIONS; a++) {
            if (prescan.FileiocRoutinesStack[a]) {
                prescan.FileiocRoutinesStack[a] = (uint24_t)ice.programPtr;
                JP(a * 3);
            }
        }
    }
    
    // Set free RAM pointers (for strings)
    prescan.freeMemoryPtr = (prescan.tempStrings[1] = (prescan.tempStrings[0] = pixelShadow + 2000 * prescan.amountOfOSVarsUsed) + 2000) + 2000;
    
    // Cleanup code
    if (prescan.hasGraphxFunctions) {
        CALL(_RunIndicOff);
        CALL(ice.programPtr - ice.programData + PRGM_START + 12);
        LD_IY_IMM(flags);
        JP(_DrawStatusBar);
    } else if (prescan.modifiedIY) {
        CALL(ice.programPtr - ice.programData + PRGM_START + 9);
        LD_IY_IMM(flags);
        RET();
    }
    
    // Check free RAM for Lbls and Gotos
    ice.LblStack = (label_t*)malloc(prescan.amountOfLbls * sizeof(label_t));
    ice.GotoStack = (label_t*)malloc(prescan.amountOfGotos * sizeof(label_t));
    if (!ice.LblStack || !ice.GotoStack) {
        return E_MEM_LABEL;
    }
    
    return VALID;
}