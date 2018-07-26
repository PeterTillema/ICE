#include "defines.h"
#include "errors.h"

#include "ast.h"
#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "functions.h"
#include "routines.h"

static const char *errors[] = {
    "This token/function is not implemented (yet)",
#ifdef CALCULATOR
    "This token cannot be used at the start of the   line",
#else
    "This token cannot be used at the start of the line",
#endif
    "This token doesn't have a condition",
    "You used 'Else' outside an If-statement",
    "You used 'End' outside a condition block",
    "You have an invalid \")\", \",\", \"(\", \")\", \"}\" or \"]\"",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Invalid hexadecimal",
    "ICE ERROR: please report it!",
    "You have the wrong number or arguments",
#ifdef CALCULATOR
    "Unknown C function. If you are sure this              function exists, please contact me!",
#else
    "Unknown C function. If you are sure this function exists, please contact me!",
#endif
    "Subprogram not found",
    "Compiling subprograms not supported",
    "Invalid program name",
    "Not enough memory for Lbl and Goto",
    "Unable to open debug file",
    "Error: not an ICE program\n",
    "Error: not enough memory for variables",
    "Warning: Unknown char in the string!",
    "Warning: string has been automatically squish-ed!",
};

void displayLabelError(char *label) {
    char buf[30];

    sprintf(buf, "Label %.13s not found", label);
#ifdef CALCULATOR
    gfx_SetTextFGColor(224);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", buf);
#endif
}

void displayError(uint8_t index) {
#ifdef CALCULATOR
    char buf[30];
    
    gfx_SetTextFGColor(index < W_WRONG_CHAR ? 224 : 227);
    displayMessageLineScroll(errors[index]);

    gfx_SetTextFGColor(0);
    sprintf(buf, "Error at line %u", ice.currentLine);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", errors[index]);
    fprintf(stdout, "Error at line %u\n", ice.currentLine);
#endif
}
