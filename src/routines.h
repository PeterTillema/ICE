#ifndef ROUTINES_H
#define ROUTINES_H

uint8_t GetIndexOfFunction(uint8_t, uint8_t);
void OutputWriteByte(uint8_t);
void OutputWriteWord(uint16_t);
void OutputWriteLong(uint24_t);
void OutputWriteMem(uint8_t mem[]);
bool IsA2ByteTok(uint8_t);
void ProgramPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
prog_t *GetProgramName(void);
void SeekMinus1(void);
void displayLoadingBar(void);
void WriteIntToDebugProg(uint24_t);
void ClearAnsFlags(void);
void LoadRegValue(uint8_t, uint24_t);
void LoadRegVariable(uint8_t, uint8_t);
void ChangeRegValue(uint24_t, uint24_t, uint8_t opcodes[7]);
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
void displayMessageLineScroll(char*);
void MaybeAToHL(void);
void MaybeLDIYFlags(void);
void CallRoutine(bool*, uint24_t*, const uint8_t*, uint8_t);
uint8_t IsHexadecimal(int);
uint8_t GetVariableOffset(uint8_t);
bool CheckEOL(void);
int getNextToken(void);
int grabString(uint8_t**, bool);
void printButton(uint24_t);

#endif
