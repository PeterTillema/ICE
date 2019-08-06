#ifndef MAIN_H
#define MAIN_H

#define AMOUNT_OF_GRAPHX_FUNCTIONS 94
#define AMOUNT_OF_FILEIOC_FUNCTIONS 34
#define AMOUNT_OF_FUNCTIONS 34

#define STACK_SIZE 500
#define SIZEOF_DISP_DATA   26
#define SIZEOF_KEYPAD_DATA 18
#define SIZEOF_RAND_DATA   118
#define SIZEOF_SRAND_DATA  17
#define SIZEOF_SQRT_DATA   43
#define SIZEOF_SINCOS_DATA 99
#define SIZEOF_MEAN_DATA   24
#define SIZEOF_OR_DATA     10
#define SIZEOF_PRGM_DATA   40
#define SIZEOF_AND_DATA    11
#define SIZEOF_XOR_DATA    13
#define SIZEOF_INPUT_DATA  96
#define SIZEOF_PAUSE_DATA  20
#define SIZEOF_MALLOC_DATA 21
#define SIZEOF_TIMER_DATA  15
#define SIZEOF_CHEADER     116

#define tDefineSprite      0x0A
#define tCall              0x0B
#define tData              0x0C
#define tCopy              0x0D
#define tAlloc             0x0E
#define tDefineTilemap     0x0F
#define tCopyData          0x10
#define tLoadData          0x11
#define tSetBrightness     0x12
#define tCompare           0x13

#define END_MALLOC         0xD13703
#define IX_VARIABLES       0xD13F56         // saveSScreen + 21945 - 130

typedef struct {
    char     name[20];
    uint8_t  *addr;
    uint16_t *debugJumpDataPtr;
    unsigned int offset;
} label_t;

typedef struct {
    char    name[21];
} variable_t;

typedef struct {
    bool     usingInputProgram;                             // Using input program from string

    char     outName[9];                                    // Output variable name

    uint8_t  *programData;                                  // Address of the program
    uint8_t  *programDataData;                              // Address of the end of the program data
    uint8_t  *programPtr;                                   // Pointer to the program
    uint8_t  *programPtrBackup;                             // Same as above
    uint8_t  *programDataPtr;                               // Pointer to the program data
    uint8_t  tempToken;                                     // Used for functions, i.e. For(, where an argument can stop with either a comma or a parentheses
    uint8_t  stackDepth;                                    // Used for compiling arguments of C functions
    
    label_t  *LblStack;                                     // Pointer to label stack
    label_t  *GotoStack;                                    // Pointer to goto stack

    unsigned int *dataOffsetStack[1000];                        // Stack of the address to point to the data, which needs to be changed after compiling
    unsigned int dataOffsetElements;                            // Amount of stack elements of above
    unsigned int dataOffsetElementsBackup;                      // Same as above
    unsigned int *ForLoopSMCStack[100];                         // Used for SMC in For loops
    unsigned int ForLoopSMCElements;                            // Amount of elements in above stack
    unsigned int currentLine;                                   // The amount of parsed lines, useful for displaying it when an error occurs
    unsigned int programSize;                                   // Size of the output program
    unsigned int *stack[STACK_SIZE*5];                          // Stacks for compiling arguments
    unsigned int *stackStart;                                   // Start of the stack
    unsigned int curLbl;                                        // Current label
    unsigned int curGoto;                                       // Current goto
    unsigned int programLength;                                 // Size of input program

    ti_var_t inPrgm;                                        // Used for getting tokens
    ti_var_t outPrgm;                                       // Used for writing bytes

    bool     lastTokenIsReturn;                             // Last token is a "Return", so we can omit our "ret" :)
    bool     modifiedIY;                                    // Some routines modify IY, and some routines needs it
    bool     debug;                                         // Used to export an appvar when debugging

    bool     usedAlreadyRand;                               // Only once the "rand" routine in the program data
    unsigned int randAddr;                                      // Address of the "rand" routine in the program data

    bool     usedAlreadyGetKeyFast;                         // Only once the "getKey(X)" routine in the program data
    unsigned int getKeyFastAddr;                                // Address of the "getKey(X)" routine in the program data

    bool     usedAlreadySqrt;                               // Only once the "sqrt(" routine in the program data
    unsigned int SqrtAddr;                                      // Address of the "sqrt(" routine in the program data

    bool     usedAlreadyMean;                               // Only once the "mean(" routine in the program data
    unsigned int MeanAddr;                                      // Address of the "mean(" routine in the program data

    bool     usedAlreadyInput;                              // Only once the "Input" routine in the program data
    unsigned int InputAddr;                                     // Address of the "Input" routine in the program data

    bool     usedAlreadyPause;                              // Only once the "Pause " routine in the program data
    unsigned int PauseAddr;                                     // Address of the "Pause " routine in the program data

    bool     usedAlreadySinCos;                             // Only once the "sin(" or "cos(" routine in the program data
    unsigned int SinCosAddr;                                    // Address of the "sin(" or "cos(" routine in the program data

    bool     usedAlreadyLoadSprite;                         // Only once the "LoadData(" routine in the program data
    unsigned int LoadSpriteAddr;                                // Address of the "LoadData(" routine in the program data

    bool     usedAlreadyLoadTilemap;                        // Only once the "LoadData(" routine in the program data
    unsigned int LoadTilemapAddr;                               // Address of the "LoadData(" routine in the program data

    bool     usedAlreadyMalloc;                             // Only once the "Alloc(" routine in the program data
    unsigned int MallocAddr;                                    // Address of the "Alloc(" routine in the program data

    bool     usedAlreadyTimer;                              // Only once the timer routine in the program data
    unsigned int TimerAddr;                                     // Address of the timer routine in the program data
    
    bool     usedAlreadyDisp;                               // Only once the Disp routine in the program data
    unsigned int DispAddr;                                      // Address of the Disp routine in the program data
    
    bool     usedAlreadyPrgm;                               // Only once the prgm routine in the program data
    unsigned int PrgmAddr;                                      // Address of the prgm routine in the program data
} ice_t;

#ifdef CALCULATOR
typedef struct {
    uint8_t  curProgIndex;                                  // Currently compiling (sub)program
    uint8_t  amountOfPrograms;                              // Count the amount of (sub)programs
    uint8_t  *jumpAddress;                                  // Used for debugging
    uint8_t  *debugLibPtr;                                  // Used for debugging
    
    unsigned int currentBreakPointLine;                         // Amount of breakpoints at startup
    unsigned int breakPointLines[100];                          // List with startup breakpoint line numbers
    unsigned int currentLine;                                   // The amount of parsed lines in the output program
    
    ti_var_t dbgPrgm;                                       // Used for writing debug things
} debug_t;

typedef struct {
    char     name[8];
    uint16_t startingLine;
    uint16_t endingLine;
    uint16_t CRC;
} debug_prog_t;

typedef struct {
    uint8_t  subprogramIndex;
    uint16_t offset;
    uint16_t localLine;
    uint16_t jumpOffset;
} line_t;
#endif

typedef struct {
    bool     inString;
    bool     inFunction;
    bool     outputIsNumber;
    bool     outputIsVariable;
    bool     outputIsString;
    bool     AnsSetZeroFlag;
    bool     AnsSetZeroFlagReversed;
    bool     AnsSetCarryFlag;
    bool     AnsSetCarryFlagReversed;

    uint8_t  ZeroCarryFlagRemoveAmountOfBytes;
    uint8_t  outputRegister;
    uint8_t  outputReturnRegister;
    uint8_t  SizeOfOutputNumber;

    unsigned int outputNumber;
} expr_t;

typedef struct {
    bool       modifiedIY;
    bool       usedTempStrings;
    bool       hasGraphxStart;
    bool       hasGraphxEnd;
    bool       hasFileiocStart;
    bool       hasGraphxFunctions;
    bool       hasFileiocFunctions;

    uint8_t    amountOfRandRoutines;
    uint8_t    amountOfSqrtRoutines;
    uint8_t    amountOfMeanRoutines;
    uint8_t    amountOfInputRoutines;
    uint8_t    amountOfPauseRoutines;
    uint8_t    amountOfSinCosRoutines;
    uint8_t    amountOfLoadSpriteRoutines;
    uint8_t    amountOfLoadTilemapRoutines;
    uint8_t    amountOfMallocRoutines;
    uint8_t    amountOfTimerRoutines;
    uint8_t    amountOfOSVarsUsed;
    uint8_t    amountOfVariablesUsed;

    unsigned int   amountOfLbls;
    unsigned int   amountOfGotos;
    unsigned int   GraphxRoutinesStack[AMOUNT_OF_GRAPHX_FUNCTIONS];
    unsigned int   FileiocRoutinesStack[AMOUNT_OF_FILEIOC_FUNCTIONS];
    unsigned int   OSStrings[10];
    unsigned int   OSLists[10];
    unsigned int   freeMemoryPtr;
    unsigned int   tempStrings[2];
    
    variable_t variables[255];
} prescan_t;

typedef struct {
    bool     HLIsNumber;
    bool     HLIsVariable;
    bool     DEIsNumber;
    bool     DEIsVariable;
    bool     BCIsNumber;
    bool     BCIsVariable;
    bool     AIsNumber;
    bool     AIsVariable;
    bool     tempBool;
    bool     allowedToOptimize;

    uint8_t  HLVariable;
    uint8_t  DEVariable;
    uint8_t  BCVariable;
    uint8_t  AValue;
    uint8_t  AVariable;
    uint8_t  tempVariable;

    unsigned int HLValue;
    unsigned int DEValue;
    unsigned int BCValue;
    unsigned int tempValue;
} reg_t;

typedef struct {
    uint8_t errorCode;
    char    prog[9];
} prog_t;

extern ice_t ice;
extern expr_t expr;
extern prescan_t prescan;
extern reg_t reg;
extern variable_t variable;

#ifdef CALCULATOR
extern line_t line;
extern debug_t debug;
extern debug_prog_t debug_prog;

void CheaderData(void);
void GraphxHeader(void);
void FileiocheaderData(void);
void ICEDebugheaderData(void);
void CProgramHeaderData(void);
void OrData(void);
void AndData(void);
void XorData(void);
void RandData(void);
void KeypadData(void);
void GetKeyFastData(void);
void GetKeyFastData2(void);
void StringStoData(void);
void InputData(void);
void MallocData(void);
void SincosData(void);
void PrgmData(void);
void TimerData(void);
void DispData(void);
void StringConcatenateData(void);
void LoadspriteData(void);
void LoadtilemapData(void);
void MeanData(void);
void SqrtData(void);
void PauseData(void);
void GotoEditor(char*, uint16_t);
void RunPrgm(char*);
void SetHooks(uint8_t*);
uint16_t GetCRC(uint8_t*, size_t);
#endif

#endif
