#ifndef ERRORS_H
#define ERRORS_H

#define E_UNIMPLEMENTED    0
#define E_WRONG_PLACE      1
#define E_NO_CONDITION     2
#define E_ELSE             3
#define E_END              4
#define E_EXTRA_RPAREN     5
#define E_SYNTAX           6
#define E_WRONG_ICON       7
#define E_INVALID_HEX      8
#define E_ICE_ERROR        9
#define E_ARGUMENTS        10
#define E_UNKNOWN_C        11
#define E_PROG_NOT_FOUND   12
#define E_NO_SUBPROG       13
#define E_INVALID_PROG     14
#define E_MEM_LABEL        15
#define E_NO_DBG_FILE      16
#define E_NOT_ICE_PROG     17
#define E_MEM_VARIABLES    18
#define E_SUBPROG_HEADER   19
#define W_WRONG_CHAR       20
#define W_SQUISHED         21

//If the user presses the break [graph] key.
#define E_BREAK            253

#define W_VALID            254
#define VALID              255

void displayLabelError(char *label);
void displayError(uint8_t);

//If the user presses the break [graph] key.
void displayBreakError(void);

#endif
