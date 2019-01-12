#ifndef ROUTINES_H
#define ROUTINES_H

#include "main.h"

uint8_t GetIndexOfFunction(uint8_t, uint8_t);

void OutputWriteByte(uint8_t);

void OutputWriteWord(uint16_t);

void OutputWriteLong(unsigned int);

void OutputWriteMem(uint8_t mem[]);

bool IsA2ByteTok(uint8_t);

void ProgramPtrToOffsetStack(void);

void displayLoadingBarFrame(void);

prog_t *GetProgramName(void);

void SeekMinus1(void);

void displayLoadingBar(void);

void WriteIntToDebugProg(unsigned int);

void WriteWordToDebugProg(unsigned int);

void ClearAnsFlags(void);

void LoadRegValue(uint8_t, unsigned int);

void LoadRegVariable(uint8_t, uint8_t);

void ChangeRegValue(unsigned int, unsigned int, uint8_t opcodes[7]);

void ResetAllRegs(void);

void ResetHL(void);

void ResetDE(void);

void ResetBC(void);

void ResetA(void);

void RegChangeHLDE(void);

void SetRegHLToRegDE(void);

void SetRegDEToRegHL(void);

void PushHLDE(void);

void AnsToHL(void);

void AnsToDE(void);

void AnsToBC(void);

void AnsToA(void);

void displayMessageLineScroll(char *);

void MaybeAToHL(void);

void MaybeLDIYFlags(void);

void CallRoutine(bool *, unsigned int *, const uint8_t *, uint8_t);

uint8_t IsHexadecimal(int);

uint8_t GetVariableOffset(uint8_t);

bool CheckEOL(void);

int getNextToken(void);

int grabString(uint8_t **, bool);

void printButton(unsigned int);

#endif
