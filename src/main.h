#ifndef MAIN_H
#define MAIN_H

#define AMOUNT_OF_GRAPHX_FUNCTIONS 94
#define AMOUNT_OF_FILEIOC_FUNCTIONS 34
#define AMOUNT_OF_FUNCTIONS 33

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
    uint8_t  *debugJumpDataPtr;
    uint24_t offset;
} label_t;

typedef struct {
    char    name[21];
} variable_t;

typedef struct {
    bool     usingInputProgram;                             // Using input program from string

    char     outName[9];                                    // Output variable name
    char     currProgName[5][9];                            // Current program compiling

    uint8_t  *programData;                                  // Address of the program
    uint8_t  *programDataData;                              // Address of the end of the program data
    uint8_t  *programPtr;                                   // Pointer to the program
    uint8_t  *programPtrBackup;                             // Same as above
    uint8_t  *programDataPtr;                               // Pointer to the program data
    uint8_t  tempToken;                                     // Used for functions, i.e. For(, where an argument can stop with either a comma or a parentheses
    uint8_t  stackDepth;                                    // Used for compiling arguments of C functions
    uint8_t  *jumpAddress;                                  // Used for debugging
    uint8_t  *debugLibPtr;                                  // Used for debugging
    
    label_t  *LblStack;                                     // Pointer to label stack
    label_t  *GotoStack;                                    // Pointer to goto stack

    uint24_t *dataOffsetStack[1000];                        // Stack of the address to point to the data, which needs to be changed after compiling
    uint24_t dataOffsetElements;                            // Amount of stack elements of above
    uint24_t dataOffsetElementsBackup;                      // Same as above
    uint24_t *ForLoopSMCStack[100];                         // Used for SMC in For loops
    uint24_t ForLoopSMCElements;                            // Amount of elements in above stack
    uint24_t currentLine;                                   // The amount of parsed lines, useful for displaying it when an error occurs
    uint24_t programSize;                                   // Size of the output program
    uint24_t *stack[STACK_SIZE*5];                          // Stacks for compiling arguments
    uint24_t *stackStart;                                   // Start of the stack
    uint24_t curLbl;                                        // Current label
    uint24_t curGoto;                                       // Current goto
    uint24_t programLength;                                 // Size of input program
    uint24_t currentBreakPointLine;                         // Used for debugging
    uint24_t breakPointLines[100];                          // Used for debugging

    ti_var_t inPrgm;                                        // Used for getting tokens
    ti_var_t outPrgm;                                       // Used for writing bytes
    ti_var_t dbgPrgm;                                       // Used for writing debug things

    bool     lastTokenIsReturn;                             // Last token is a "Return", so we can omit our "ret" :)
    bool     modifiedIY;                                    // Some routines modify IY, and some routines needs it
    bool     debug;                                         // Used to export an appvar when debugging

    bool     usedAlreadyRand;                               // Only once the "rand" routine in the program data
    uint24_t randAddr;                                      // Address of the "rand" routine in the program data

    bool     usedAlreadyGetKeyFast;                         // Only once the "getKey(X)" routine in the program data
    uint24_t getKeyFastAddr;                                // Address of the "getKey(X)" routine in the program data

    bool     usedAlreadySqrt;                               // Only once the "sqrt(" routine in the program data
    uint24_t SqrtAddr;                                      // Address of the "sqrt(" routine in the program data

    bool     usedAlreadyMean;                               // Only once the "mean(" routine in the program data
    uint24_t MeanAddr;                                      // Address of the "mean(" routine in the program data

    bool     usedAlreadyInput;                              // Only once the "Input" routine in the program data
    uint24_t InputAddr;                                     // Address of the "Input" routine in the program data

    bool     usedAlreadyPause;                              // Only once the "Pause " routine in the program data
    uint24_t PauseAddr;                                     // Address of the "Pause " routine in the program data

    bool     usedAlreadySinCos;                             // Only once the "sin(" or "cos(" routine in the program data
    uint24_t SinCosAddr;                                    // Address of the "sin(" or "cos(" routine in the program data

    bool     usedAlreadyLoadSprite;                         // Only once the "LoadData(" routine in the program data
    uint24_t LoadSpriteAddr;                                // Address of the "LoadData(" routine in the program data

    bool     usedAlreadyLoadTilemap;                        // Only once the "LoadData(" routine in the program data
    uint24_t LoadTilemapAddr;                               // Address of the "LoadData(" routine in the program data

    bool     usedAlreadyMalloc;                             // Only once the "Alloc(" routine in the program data
    uint24_t MallocAddr;                                    // Address of the "Alloc(" routine in the program data

    bool     usedAlreadyTimer;                              // Only once the timer routine in the program data
    uint24_t TimerAddr;                                     // Address of the timer routine in the program data
    
    bool     usedAlreadyDisp;                               // Only once the Disp routine in the program data
    uint24_t DispAddr;                                      // Address of the Disp routine in the program data
    
    bool     usedAlreadyPrgm;                               // Only once the prgm routine in the program data
    uint24_t PrgmAddr;                                      // Address of the prgm routine in the program data
} ice_t;

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

    uint24_t outputNumber;
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

    uint24_t   amountOfLbls;
    uint24_t   amountOfGotos;
    uint24_t   GraphxRoutinesStack[AMOUNT_OF_GRAPHX_FUNCTIONS];
    uint24_t   FileiocRoutinesStack[AMOUNT_OF_FILEIOC_FUNCTIONS];
    uint24_t   OSStrings[10];
    uint24_t   OSLists[10];
    uint24_t   freeMemoryPtr;
    uint24_t   tempStrings[2];
    
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

    uint24_t HLValue;
    uint24_t DEValue;
    uint24_t BCValue;
    uint24_t tempValue;
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
#endif

#endif
