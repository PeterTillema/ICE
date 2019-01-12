#ifndef PARSE_H
#define PARSE_H

// In case I want to add more types, be sure that TYPE_STRING will be the last one, otherwise operator.c will mess up
enum {
    TYPE_NUMBER,
    TYPE_VARIABLE,
    TYPE_CHAIN_ANS,
    TYPE_CHAIN_PUSH,
    TYPE_STRING
};

enum {
    TYPE_C_START = 124,
    TYPE_ARG_DELIMITER,
    TYPE_OPERATOR,
    TYPE_FUNCTION
};

enum {
    TYPE_MASK_U8,
    TYPE_MASK_U16,
    TYPE_MASK_U24
};

bool JumpForward(uint8_t *, uint8_t *, unsigned int, uint8_t, uint8_t);

bool JumpBackwards(uint8_t *, uint8_t);

void optimizeZeroCarryFlagOutput(void);

void skipLine(void);

void insertGotoLabel(void);

uint8_t parsePostFixFromIndexToIndex(unsigned int, unsigned int);

uint8_t functionRepeat(int);

uint8_t parseProgram(void);

uint8_t parseProgramUntilEnd(void);

#endif
