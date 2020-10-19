#include "defines.h"
#include "main.h"

#ifdef CALCULATOR

#include "ast.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "routines.h"
#include "prescan.h"

#define DEBUG_VERSION_MAJOR 0
#define DEBUG_VERSION_MINOR 1

#define NUMBEROFPROGRAM 256
#define PROGRAMPERSCREEN 21

ice_t ice;
expr_t expr;
reg_t reg;

const char *infoStr = "ICE Compiler v2.2.1 - By Peter \"PT_\" Tillema";
static char *inputPrograms[NUMBEROFPROGRAM];

static int myCompare(const void * a, const void * b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void displayProgramList(unsigned int beginList, unsigned int amountOfProgramsToDisplay) {
    unsigned int i;
    
    for (i = 0; i < amountOfProgramsToDisplay; i++) {
        gfx_PrintStringXY(inputPrograms[beginList + i], 10, i * 10 + 13);
    }
}

void clearProgramList() {
    unsigned int i;
    
    for (i = 0; i < PROGRAMPERSCREEN; i++) {
        gfx_FillRectangle_NoClip(10, i * 10 + 13, 200, 10);
    }
}

void main(void) {
    uint8_t inputProgramType;
    char *inputProgram;
    uint16_t inputProgramSize;
    ti_var_t programSlot;
    uint8_t selectedProgram, amountOfPrograms, res = VALID, type;
    unsigned int programDataSize, offset, totalSize;
    uint8_t beginList, amountOfProgramsToDisplay;
    uint8_t relativeSelectedProgram;
    const char ICEheader[] = {tii, 0};
    ti_var_t tempProg;
    char buf[30], *temp_name = "", var_name[9];
    sk_key_t key = 0;
    void *search_pos;
    bool didCompile;

    // Install hooks
    ti_CloseAll();
    if ((tempProg = ti_Open("ICEHOOKS", "r"))) {
        ti_SetArchiveStatus(true, tempProg);
        SetHooks(ti_GetDataPtr(tempProg));
    }
    
    // Enable lowercase
    asm("ld iy, 0D00080h");
    asm("set 3, (iy+024h)");

    // check if a program was used as input
    ti_CloseAll();
    ice.usingInputProgram = false;
    inputProgram = os_GetAnsData(&inputProgramType);
    if (inputProgram && inputProgramType == TI_STRING_TYPE && inputProgram[2] == tProg && (inputProgramSize = *(uint16_t*)inputProgram) < 10) {
        memset(var_name, 0, sizeof var_name);
        memcpy(var_name, inputProgram + 3, inputProgramSize - 1);
        programSlot = ti_OpenVar(var_name, "r", TI_PRGM_TYPE);
        if (programSlot) {
            ice.usingInputProgram = true;
        }
    }

    // Yay, GUI! :)
displayMainScreen:
    gfx_Begin();

    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_SetTextFGColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY(infoStr, 12, 1);

    // Get all the programs that start with the [i] token
    selectedProgram = 0;
    didCompile = false;
    ti_CloseAll();

    if (ice.usingInputProgram) {
        goto compile_program;
    }

    search_pos = NULL;
    while ((temp_name = ti_DetectAny(&search_pos, ICEheader, &type)) != NULL) {
        if (type == TI_PRGM_TYPE || type == TI_PPRGM_TYPE) {
            // Hidden programs
            if ((uint8_t)(*temp_name) < 64) {
                *temp_name += 64;
            }

            // Save the program name
            inputPrograms[selectedProgram] = malloc(9);
            strcpy(inputPrograms[selectedProgram++], temp_name);
        }
        
        if (selectedProgram >= NUMBEROFPROGRAM) {
            break;
        }
    }

    amountOfPrograms = selectedProgram;
    beginList = 0;
    amountOfProgramsToDisplay = (amountOfPrograms > PROGRAMPERSCREEN ? PROGRAMPERSCREEN : amountOfPrograms);

    // Check if there are ICE programs
    if (!amountOfPrograms) {
        gfx_PrintStringXY("No programs found!", 10, 13);
        goto stop;
    }

    // Display all the sorted programs
    qsort(inputPrograms, amountOfPrograms, sizeof(char *), myCompare);
    displayProgramList(beginList, amountOfProgramsToDisplay);
    
    // Display buttons
    gfx_PrintStringXY("Build", 4, 232);
    printButton(1);
    gfx_PrintStringXY("Debug", 66, 232);
    printButton(65);
    gfx_PrintStringXY("Quit", 285, 232);
    printButton(279);
    gfx_SetColor(0);

    // Select a program
    selectedProgram = 1;
    relativeSelectedProgram = 1;
    while ((key = os_GetCSC()) != sk_Enter && key != sk_2nd && key != sk_Yequ && key != sk_Window) {
        uint8_t selectionOffset = relativeSelectedProgram * 10 + 3;

        gfx_PrintStringXY(">", 1, selectionOffset);

        if (key) {
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(1, selectionOffset, 8, 8);

            // Stop and quit
            if (key == sk_Clear || key == sk_Graph) {
                goto err;
            }

            // Select the next program
            if (key == sk_Down) {
                if (selectedProgram != amountOfPrograms) {
                    selectedProgram++;
                    relativeSelectedProgram++;
                    if (relativeSelectedProgram > PROGRAMPERSCREEN) {
                        clearProgramList();
                        relativeSelectedProgram--;
                        beginList++;
                        displayProgramList(beginList, amountOfProgramsToDisplay);
                    }
                } else {
                    clearProgramList();
                    selectedProgram = 1;
                    relativeSelectedProgram = 1;
                    beginList = 0;
                    displayProgramList(beginList, amountOfProgramsToDisplay);
                }
            }

            // Select the previous program
            if (key == sk_Up) {
                if (selectedProgram != 1) {
                    selectedProgram--;
                    relativeSelectedProgram--;
                    if(relativeSelectedProgram == 0) {
                        clearProgramList();
                        relativeSelectedProgram++;
                        beginList--;
                        displayProgramList(beginList, amountOfProgramsToDisplay);
                    }
                } else {
                    clearProgramList();
                    selectedProgram = amountOfPrograms;
                    relativeSelectedProgram = (amountOfPrograms > PROGRAMPERSCREEN ? PROGRAMPERSCREEN : amountOfPrograms);
                    beginList = (selectedProgram >= PROGRAMPERSCREEN ? selectedProgram - PROGRAMPERSCREEN : 0);
                    displayProgramList(beginList, amountOfProgramsToDisplay);
                }
            }
        }
    }
    
    // Set some vars
    strcpy(var_name, inputPrograms[selectedProgram - 1]);
    for (selectedProgram = 0; selectedProgram < amountOfPrograms; selectedProgram++) {
        free(inputPrograms[selectedProgram]);
    }

compile_program:

    // Erase screen
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, 11, 320, 210);
    gfx_FillRectangle_NoClip(0, 220, 270, 20);

    didCompile = true;
    memset(&ice, 0, sizeof ice);
    memset(&expr, 0, sizeof expr);
    memset(&reg, 0, sizeof reg);
    memset(&prescan, 0, sizeof prescan);
    memset(&debug, 0, sizeof debug);
    
    // Output debug appvar
    if (key == sk_Window) {
        ice.debug = true;
    }

    gfx_SetTextXY(1, 12);
    displayMessageLineScroll("Prescanning...");
    displayLoadingBarFrame();

    ice.inPrgm = _open(var_name);
    _seek(0, SEEK_END, ice.inPrgm);

    ice.programLength   = _tell(ice.inPrgm);
    ice.programData     = (uint8_t*)0xD52C00;
    ice.programPtr      = ice.programData;
    ice.programDataData = ice.programData + 0xFFFF;
    ice.programDataPtr  = ice.programDataData;
    
    // Get the name/icon/description
    _rewind(ice.inPrgm);
    if ((res = getNameIconDescription()) != VALID) {
        displayError(res);
        goto stop;
    }
    
    // Open debug appvar to store things to
    sprintf(buf, "%.5sDBG", ice.outName);
    debug.dbgPrgm = ti_Open(buf, "w");
    
    if (ice.debug) {
        if (!debug.dbgPrgm) {
            displayError(E_NO_DBG_FILE);
            goto stop;
        }
        
        // Write version bytes to debug appvar
        ti_PutC(DEBUG_VERSION_MAJOR, debug.dbgPrgm);
        ti_PutC(DEBUG_VERSION_MINOR, debug.dbgPrgm);
        
        // Write amount of programs to debug appvar
        ti_PutC(0, debug.dbgPrgm);
    } else if (debug.dbgPrgm) {
        ti_Delete(buf);
    }

    // Prescan the program and output the header
    preScanProgram();
    if ((res = parsePrescan()) != VALID) {
        displayError(res);
        goto stop;
    }
	
	if (prescan.amountOfVariablesUsed > 84) {
		gfx_SetTextFGColor(224);
		sprintf(buf, "Too much variables used: %d", prescan.amountOfVariablesUsed);
		displayMessageLineScroll(buf);
		didCompile = false;
		goto stop;
	}

    // Allow hidden programs from Cesium
    if (*var_name < 64) {
        *var_name += 64;
    }
    sprintf(buf, "Compiling program %s...", var_name);
    displayMessageLineScroll(buf);

    // Create or empty the output program if parsing succeeded
    if ((res = parseProgram()) == VALID) {
        unsigned int previousSize = 0;

        // Get the sizes of both stacks
        ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
        programDataSize = (uintptr_t)ice.programDataData - (uintptr_t)ice.programDataPtr;

        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uintptr_t)ice.programDataPtr;
        while (ice.dataOffsetElements--) {
            unsigned int *tempDataOffsetStackPtr = ice.dataOffsetStack[ice.dataOffsetElements];

            *tempDataOffsetStackPtr += offset;
        }
        totalSize = ice.programSize + programDataSize + 3;

        // Export the program
        ice.outPrgm = _open(ice.outName);
        if (ice.outPrgm) {
            // This program already exists
            if ((uint8_t)ti_GetC(ice.outPrgm) != 0xEF || (uint8_t)ti_GetC(ice.outPrgm) != 0x7B) {
                gfx_SetTextFGColor(224);
                displayMessageLineScroll("Output program already exists!");
                displayMessageLineScroll("Delete program to continue.");
                didCompile = false;
                goto stop;
            }
            
            previousSize = ti_GetSize(ice.outPrgm);
            ti_Close(ice.outPrgm);
        }
        ice.outPrgm = _new(ice.outName);
        if (!ice.outPrgm) {
            displayMessageLineScroll("Failed to open output file");
            goto stop;
        }

        // Write ASM header
        ti_PutC(tExtTok, ice.outPrgm);
        ti_PutC(tAsm84CeCmp, ice.outPrgm);

        // Write ICE header to be recognized by Cesium
        ti_PutC(0x7F, ice.outPrgm);

        // Write the header, main program, and data to output :D
        ti_Write(ice.programData, ice.programSize, 1, ice.outPrgm);
        if (programDataSize) ti_Write(ice.programDataPtr, programDataSize, 1, ice.outPrgm);
        _rewind(ice.outPrgm);
        
        // Write final CRC to debug program, as well as the ending line of the first program and the amount of total programs
        if (ice.debug) {
            uint16_t CRC;
            
            CRC = GetCRC(ti_GetDataPtr(ice.outPrgm), ti_GetSize(ice.outPrgm));
            WriteWordToDebugProg(CRC);
            ti_Seek(3 + offsetof(debug_prog_t, endingLine), SEEK_SET, debug.dbgPrgm);
            WriteWordToDebugProg(debug.currentLine);
            ti_Seek(2, SEEK_SET, debug.dbgPrgm);
            ti_PutC(debug.amountOfPrograms + 1, debug.dbgPrgm);         // +1 because the main program starts at 0
        }

        // Yep, we are really done!
        gfx_SetTextFGColor(4);
        displayMessageLineScroll("Successfully compiled!");
        if (ice.debug) {
            displayMessageLineScroll("Successfully exported debug appvar");
        }

        // Skip line
        displayMessageLineScroll(" ");

        // Display the size
        gfx_SetTextFGColor(0);
        sprintf(buf, "Output size: %u bytes", totalSize);
        displayMessageLineScroll(buf);
        if (previousSize) {
            sprintf(buf, "Previous size: %u bytes", previousSize);
            displayMessageLineScroll(buf);
        }
        sprintf(buf, "Output program: %s", ice.outName);
        displayMessageLineScroll(buf);
    } else if (res != W_VALID) {
#ifdef CALCULATOR
        if (res==E_BREAK){
            displayBreakError();
        } else {
#endif
            displayError(res);
#ifdef CALCULATOR
        }
#endif
    }

stop:
    gfx_SetTextFGColor(0);
    if (didCompile) {
        if (res == VALID) {
            gfx_PrintStringXY("Run", 9, 232);
            printButton(1);
        } else if (!ti_IsArchived(ice.inPrgm)) {
            gfx_PrintStringXY("Goto", 222, 232);
            printButton(217);
        }
        gfx_PrintStringXY("Back", 70, 232);
        printButton(65);
        gfx_PrintStringXY("Quit", 285, 232);
        printButton(279);
    }
    while (!(key = os_GetCSC()));
err:
    gfx_End();

    if (key != sk_Graph && didCompile) {
        if (key == sk_Yequ && res == VALID) {
            RunPrgm(ice.outName);
        }
        if (key == sk_Window) {
            // Erase screen
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(0, 11, 320, 229);

            goto displayMainScreen;
        }
        if (key == sk_Trace && res != VALID && !ti_IsArchived(ice.inPrgm)) {
            char buf[9];
            
            ti_GetName(buf, ice.inPrgm);
            GotoEditor(buf, ti_Tell(ice.inPrgm) - 1);
        }
    }
    ti_CloseAll();
}

#endif
