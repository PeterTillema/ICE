#include "defines.h"
#include "main.h"

#ifndef CALCULATOR

#include "ast.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "routines.h"
#include "prescan.h"

ice_t ice;
expr_t expr;
reg_t reg;

const char *infoStr = "ICE Compiler v2.2.0.0 - By Peter \"PT_\" Tillema";

extern const uint8_t CheaderData[];
extern const uint8_t RandData[];
extern const uint8_t SrandData[];
extern const uint8_t FileiocheaderData[];

extern char *str_dupcat(const char *s, const char *c);

void w24(void *x, uint32_t val) {
    uint8_t *ptr = (uint8_t*)(x);
    ptr[0] = val & 0xFF;
    ptr[1] = val >> 8 & 0xFF;
    ptr[2] = val >> 16 & 0xFF;
}

void w16(void *x, uint32_t val) {
    uint8_t *ptr = (uint8_t*)(x);
    ptr[0] = val & 0xFF;
    ptr[1] = val >> 8 & 0xFF;
}

uint32_t r24(void *x) {
    uint8_t *ptr = (uint8_t*)(x);
    return (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]);
}

int main(int argc, char **argv) {
    uint8_t amountOfPrograms, res = VALID, temp;
    uint24_t programDataSize, offset, totalSize;
    char buf[30], *var_name = argv[1];
    prog_t *outputPrgm;

    if (argc == 1) {
        fprintf(stdout, "%s\n", infoStr);
        fprintf(stdout, "Usage: ice INPUT.8xp");
        exit(0);
    } else if (argc != 2) {
        fprintf(stderr, "Error: invalid amount of arguments\n");
        exit(1);
    }

    ice.inPrgm = _open(var_name);
    if (!ice.inPrgm) {
        fprintf(stdout, "Can't find input program\n");
        goto stop;
    }
    fprintf(stdout, "%s\nPrescanning...\n", infoStr);
    _seek(0, SEEK_END, ice.inPrgm);

    ice.programLength   = _tell(ice.inPrgm);
    ice.programData     = malloc(0xFFFF + 0x100);
    ice.programPtr      = ice.programData;
    ice.programDataData = ice.programData + 0xFFFF;
    ice.programDataPtr  = ice.programDataData;

    // Check for icon and description before putting the C functions in the output program
    preScanProgram();
    if ((res = getNameIconDescription()) != VALID || (res = parsePrescan()) != VALID) {
        displayError(res);
        goto stop;
    }

    // Do the stuff
    fprintf(stdout, "Compiling program %s...\n", var_name);
    
    // Create or empty the output program if parsing succeeded
    if ((res = parseProgram()) == VALID) {
        uint24_t previousSize = 0;
        
        // Get the sizes of both stacks
        ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
        programDataSize = (uintptr_t)ice.programDataData - (uintptr_t)ice.programDataPtr;
        
        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uintptr_t)ice.programDataPtr;
        while (ice.dataOffsetElements--) {
            w24(ice.dataOffsetStack[ice.dataOffsetElements], r24(ice.dataOffsetStack[ice.dataOffsetElements]) + offset);
        }
        totalSize = ice.programSize + programDataSize + 3;

        uint8_t *export = malloc(0x10000);

        // Write ASM header
        export[0] = tExtTok;
        export[1] = tAsm84CeCmp;

        // Write ICE header to be recognized by Cesium
        export[2] = 0x7F;

        // Write the header, main program, and data to output :D
        memcpy(&export[3], ice.programData, ice.programSize);
        memcpy(&export[3 + ice.programSize], ice.programDataPtr, programDataSize);

        // Write the actual program file
        export_program(ice.outName, export, totalSize);
        free(export);
        free(ice.programData);

        // Display the size
        fprintf(stdout, "Succesfully compiled to %s.8xp!\n", ice.outName);
        fprintf(stdout, "Output size: %u bytes\n", totalSize);
    } else if (res != W_VALID) {
        displayError(res);
    }
    return 0;
stop:
    return 1;
}

#endif
