#ifndef OUTPUT_H
#define OUTPUT_H

#include "defines.h"
#include "main.h"

#define PRGM_START        0xD1A882
#define flags             0xD00080
#define curRow            0xD00595
#define curCol            0xD00596
#define OP1               0xD005F8
#define pixelShadow       0xD031F6
#define mpBlLevel         0xF60024

#define _GetCSC           0x02014C
#define _Mov9ToOP1        0x020320
#define _PutS             0x0207C0
#define _NewLine          0x0207F0
#define _ClrLCDFull       0x020808
#define _HomeUp           0x020828
#define _RunIndicOff      0x020848
#define _ParseInp         0x020F00
#define _RclAns           0x020F50
#define _ConvOP1          0x020F70
#define _DrawStatusBar    0x021A3C
#define _os_GetCSC        0x021D3C
#define _SetHLUTo0        0x021D8C
#define _DispHL           0x021EE0

#define __strcat          0x0000C0
#define __strcmp          0x0000C8
#define __strcpy          0x0000CC
#define __strlen          0x0000D4
#define __strstr          0x0000F0
#define __iand            0x000134
#define __idvrmu          0x000144
#define __imul_b          0x000150
#define __imuls           0x000154
#define __ineg            0x000160
#define __ior             0x000168
#define __ixor            0x000198

#define OP_LD_BC      0x01
#define OP_LD_B       0x06
#define OP_LD_C       0x0E
#define OP_DJNZ       0x10
#define OP_LD_DE      0x11
#define OP_INC_DE     0x13
#define OP_INC_D      0x14
#define OP_DEC_D      0x15
#define OP_LD_D       0x16
#define OP_JR         0x18
#define OP_ADD_HL_DE  0x19
#define OP_LD_A_DE    0x1A
#define OP_DEC_DE     0x1B
#define OP_LD_E       0x1E
#define OP_JR_NZ      0x20
#define OP_LD_HL      0x21
#define OP_INC_HL     0x23
#define OP_INC_H      0x24
#define OP_DEC_H      0x25
#define OP_LD_H       0x26
#define OP_JR_Z       0x28
#define OP_ADD_HL_HL  0x29
#define OP_LD_HL_IND  0x2A
#define OP_DEC_HL     0x2B
#define OP_LD_L       0x2E
#define OP_JR_NC      0x30
#define OP_LD_IMM_A   0x32
#define OP_SCF        0x37
#define OP_JR_C       0x38
#define OP_INC_A      0x3C
#define OP_DEC_A      0x3D
#define OP_LD_A       0x3E
#define OP_CCF        0x3F
#define OP_LD_B_A     0x47
#define OP_LD_C_A     0x4F
#define OP_LD_D_A     0x57
#define OP_LD_E_A     0x5F
#define OP_LD_H_A     0x67
#define OP_LD_L_A     0x6F
#define OP_LD_HL_C    0x71
#define OP_LD_HL_D    0x72
#define OP_LD_HL_E    0x73
#define OP_LD_HL_A    0x77
#define OP_LD_A_B     0x78
#define OP_LD_A_D     0x7A
#define OP_LD_A_E     0x7B
#define OP_LD_A_H     0x7C
#define OP_LD_A_L     0x7D
#define OP_LD_A_HL    0x7E
#define OP_ADD_A_L    0x85
#define OP_ADD_A_A    0x87
#define OP_ADC_A_H    0x8C
#define OP_SUB_A_D    0x92
#define OP_SUB_A_L    0x95
#define OP_SBC_A_A    0x9F
#define OP_AND_A_L    0xA5
#define OP_XOR_A_L    0xAD
#define OP_XOR_A_A    0xAF
#define OP_OR_A_C     0xB1
#define OP_OR_A_D     0xB2
#define OP_OR_A_A     0xB7
#define OP_CP_A_A     0xBF
#define OP_RET_NZ     0xC0
#define OP_POP_BC     0xC1
#define OP_JP_NZ      0xC2
#define OP_JP         0xC3
#define OP_CALL_NZ    0xC4
#define OP_PUSH_BC    0xC5
#define OP_ADD_A      0xC6
#define OP_RET_Z      0xC8
#define OP_RET        0xC9
#define OP_CALL       0xCD
#define OP_JP_Z       0xCA
#define OP_RET_NC     0xD0
#define OP_POP_DE     0xD1
#define OP_JP_NC      0xD2
#define OP_PUSH_DE    0xD5
#define OP_SUB_A      0xD6
#define OP_RET_C      0xD8
#define OP_JP_C       0xDA
#define OP_POP_HL     0xE1
#define OP_EX_SP_HL   0xE3
#define OP_PUSH_HL    0xE5
#define OP_AND_A      0xE6
#define OP_JP_HL      0xE9
#define OP_EX_DE_HL   0xEB
#define OP_XOR_A      0xEE
#define OP_OR_A       0xF6
#define OP_CP_A       0xFE

#define OutputWrite2Bytes(x, y) OutputWriteWord((y << 8) + x)
#define OutputWrite3Bytes(x, y, z) OutputWriteLong((z << 16) + (y << 8) + x)

#define LD_A_IND_IX_OFF(off)  do { LoadRegVariable(REGISTER_A, off); } while (0)
#define LD_BC_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_BC, off); } while (0)
#define LD_DE_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_DE, off); } while (0)
#define LD_HL_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_HL, off); } while (0)
#define LD_IY_IND_IX_OFF(off) do { OutputWrite2Bytes(0xDD, 0x31); OutputWriteByte(off); } while (0)
#define LD_IX_OFF_IND_DE(off) do { OutputWrite2Bytes(0xDD, 0x1F); OutputWriteByte(off); reg.DEIsVariable = true; reg.DEVariable = off; } while (0)
#define LD_IX_OFF_IND_HL(off) do { OutputWrite2Bytes(0xDD, 0x2F); OutputWriteByte(off); reg.HLIsVariable = true; reg.HLVariable = off; } while (0)
#define LD_IX_OFF_IND_H(off)  do { OutputWrite2Bytes(0xDD, 0x74); OutputWriteByte(off); ResetHL(); } while (0)
#define LD_IX_OFF_IND_L(off)  do { OutputWrite2Bytes(0xDD, 0x75); OutputWriteByte(off); ResetHL(); } while (0)
#define LD_IX_IMM(val)        do { OutputWrite2Bytes(0xDD, 0x21); OutputWriteLong(val); } while (0)
#define LD_IY_IMM(val)        do { OutputWrite2Bytes(0xFD, 0x21); OutputWriteLong(val); } while (0)
#define LEA_HL_IY_OFF(off)    do { OutputWrite2Bytes(0xED, 0x23); OutputWriteByte(off); ResetHL(); } while (0)

#define LD_BC_IMM(val,type)   do { LoadRegValue(REGISTER_BC, val, type); } while (0)
#define LD_DE_IMM(val,type)   do { LoadRegValue(REGISTER_DE, val, type); } while (0)
#define LD_HL_IMM(val,type)   do { LoadRegValue(REGISTER_HL, val, type); } while (0)
#define LD_HL_IND(val)        do { OutputWriteByte(OP_LD_HL_IND); OutputWriteLong(val); ResetHL(); } while (0)
#define LD_IMM_A(val)         do { OutputWriteByte(OP_LD_IMM_A); OutputWriteLong(val); } while (0)
#define LD_SIS_IMM_HL(val)    do { OutputWrite2Bytes(0x40, 0x22); OutputWriteWord(val); } while (0)
#define LD_SIS_HL(val)        do { OutputWrite2Bytes(0x40, OP_LD_HL); OutputWriteWord(val); ResetHL(); } while (0)
#define LD_HL_ADDR(val)       do { OutputWriteByte(OP_LD_HL_IND); OutputWriteLong(val); ResetHL(); } while (0)
#define LD_ADDR_HL(val)       do { OutputWriteByte(0x22); OutputWriteLong(val); } while (0)
#define LD_ADDR_DE(val)       do { OutputWrite2Bytes(0xED, 0x53); OutputWriteLong(val); } while (0)
#define LD_A_ADDR(val)        do { OutputWriteByte(0x3A); OutputWriteLong(val); ResetA(); } while (0)
#define LD_ADDR_A(val)        do { OutputWriteByte(0x32); OutputWriteLong(val); } while (0)
#define LD_HL_HL()            do { OutputWrite2Bytes(0xED, 0x27); ResetHL(); } while (0)
#define LD_HL_DE()            do { OutputWrite2Bytes(0xED, 0x1F); ResetHL(); } while (0)
#define LD_HL_VAL(val)        do { OutputWriteByte(0x36); OutputWriteByte(val); } while (0)

#define LD_A(val)             do { OutputWriteByte(OP_LD_A); OutputWriteByte(val); reg.AIsNumber = true; reg.AIsVariable = false; reg.AValue = val; } while (0)
#define LD_B(val)             do { OutputWriteByte(OP_LD_B); OutputWriteByte(val); ResetBC(); } while (0)
#define LD_C(val)             do { OutputWriteByte(OP_LD_C); OutputWriteByte(val); ResetBC(); } while (0)
#define LD_H(val)             do { OutputWriteByte(OP_LD_H); OutputWriteByte(val); ResetHL(); } while (0)
#define LD_L(val)             do { OutputWriteByte(OP_LD_L); OutputWriteByte(val); ResetHL(); } while (0)
#define LD_HL_A()             do { OutputWriteByte(OP_LD_HL_A); } while (0)
#define LD_HL_C()             do { OutputWriteByte(OP_LD_HL_C); } while (0)
#define LD_HL_D()             do { OutputWriteByte(OP_LD_HL_D); } while (0)
#define LD_HL_E()             do { OutputWriteByte(OP_LD_HL_E); } while (0)
#define LD_A_DE()             do { OutputWriteByte(OP_LD_A_DE); ResetA(); } while (0)
#define LD_A_HL()             do { OutputWriteByte(OP_LD_A_HL); ResetA(); } while (0)
#define LD_B_A()              do { OutputWriteByte(OP_LD_B_A); ResetBC(); } while (0)
#define LD_C_A()              do { OutputWriteByte(OP_LD_C_A); ResetBC(); } while (0)
#define LD_D_A()              do { OutputWriteByte(OP_LD_D_A); ResetDE(); } while (0)
#define LD_E_A()              do { OutputWriteByte(OP_LD_E_A); ResetDE(); } while (0)
#define LD_H_A()              do { OutputWriteByte(OP_LD_H_A); ResetHL(); } while (0)
#define LD_L_A()              do { OutputWriteByte(OP_LD_L_A); ResetHL(); } while (0)
#define LD_A_B()              do { OutputWriteByte(OP_LD_A_B); ResetA(); } while (0)
#define LD_A_D()              do { OutputWriteByte(OP_LD_A_D); ResetA(); } while (0)
#define LD_A_E()              do { OutputWriteByte(OP_LD_A_E); ResetA(); } while (0)
#define LD_A_H()              do { OutputWriteByte(OP_LD_A_H); ResetA(); } while (0)
#define LD_A_L()              do { OutputWriteByte(OP_LD_A_L); ResetA(); } while (0)
#define INC_H()               do { OutputWriteByte(OP_INC_H); ResetHL(); } while (0)
#define DEC_H()               do { OutputWriteByte(OP_DEC_H); ResetHL(); } while (0)
#define INC_D()               do { OutputWriteByte(OP_INC_D); ResetHL(); } while (0)
#define DEC_D()               do { OutputWriteByte(OP_DEC_D); ResetHL(); } while (0)

#define INC_DE()              do { OutputWriteByte(OP_INC_DE); ResetDE(); } while (0)
#define INC_HL()              do { OutputWriteByte(OP_INC_HL); ResetHL(); } while (0)
#define DEC_DE()              do { OutputWriteByte(OP_DEC_DE); ResetDE(); } while (0)
#define DEC_HL()              do { OutputWriteByte(OP_DEC_HL); ResetHL(); } while (0)
#define ADD_HL_DE()           do { OutputWriteByte(OP_ADD_HL_DE); ResetHL(); } while (0)
#define ADD_HL_HL()           do { OutputWriteByte(OP_ADD_HL_HL); ResetHL(); } while (0)
#define SBC_HL_BC()           do { OutputWrite2Bytes(0xED, 0x42); ResetHL(); } while (0)
#define SBC_HL_DE()           do { OutputWrite2Bytes(0xED, 0x52); ResetHL(); } while (0)
#define SBC_HL_HL()           do { OutputWrite2Bytes(0xED, 0x62); ResetHL(); } while (0)
#define MLT_HL()              do { OutputWrite2Bytes(0xED, 0x6C); ResetHL(); } while (0)

#define OR_A_SBC_HL_DE()      do { OutputWrite3Bytes(OP_OR_A_A, 0xED, 0x52); ResetHL(); } while (0)
#define SBC_HL_HL_INC_HL()    do { OutputWrite3Bytes(0xED, 0x62, OP_INC_HL); ResetHL(); } while (0)

#define PUSH_BC()             do { OutputWriteByte(OP_PUSH_BC); } while (0)
#define PUSH_DE()             do { OutputWriteByte(OP_PUSH_DE); } while (0)
#define PUSH_HL()             do { OutputWriteByte(OP_PUSH_HL); } while (0)
#define POP_BC()              do { OutputWriteByte(OP_POP_BC); ResetBC(); } while (0)
#define POP_DE()              do { OutputWriteByte(OP_POP_DE); ResetDE(); } while (0)
#define POP_HL()              do { OutputWriteByte(OP_POP_HL); ResetHL(); } while (0)

#define EX_DE_HL()            do { OutputWriteByte(OP_EX_DE_HL); RegChangeHLDE(); } while (0)
#define EX_S_DE_HL()          do { OutputWrite2Bytes(0x52, 0xEB); RegChangeHLDE(); } while (0)
#define EX_SP_HL()            do { OutputWriteByte(OP_EX_SP_HL); ResetHL(); } while (0)

#define RET()                 do { OutputWriteByte(OP_RET); } while (0)
#define RET_Z()               do { OutputWriteByte(OP_RET_Z); } while (0)
#define RET_NZ()              do { OutputWriteByte(OP_RET_NZ); } while (0)
#define RET_C()               do { OutputWriteByte(OP_RET_C); } while (0)
#define RET_NC()              do { OutputWriteByte(OP_RET_NC); } while (0)
#define JP(addr)              do { OutputWriteByte(OP_JP); OutputWriteLong(addr); } while (0)
#define JP_Z(addr)            do { OutputWriteByte(OP_JP_Z); OutputWriteLong(addr); } while (0)
#define JP_NZ(addr)           do { OutputWriteByte(OP_JP_NZ); OutputWriteLong(addr); } while (0)
#define JP_C(addr)            do { OutputWriteByte(OP_JP_C); OutputWriteLong(addr); } while (0)
#define JP_NC(addr)           do { OutputWriteByte(OP_JP_NC); OutputWriteLong(addr); } while (0)
#define CALL(addr)            do { OutputWriteByte(OP_CALL); OutputWriteLong(addr); } while (0)
#define CALL_NZ(addr)         do { OutputWriteByte(OP_CALL_NZ); OutputWriteLong(addr); } while (0)
#define JR_NZ(off)            do { OutputWriteByte(OP_JR_NZ); OutputWriteByte(off); } while (0)
#define JR_Z(off)             do { OutputWriteByte(OP_JR_Z); OutputWriteByte(off); } while (0)
#define JR_NC(off)            do { OutputWriteByte(OP_JR_NC); OutputWriteByte(off); } while (0)
#define JR_C(off)             do { OutputWriteByte(OP_JR_C); OutputWriteByte(off); } while (0)
#define DJNZ(off)             do { OutputWriteByte(OP_DJNZ); OutputWriteByte(off); ResetBC(); } while (0)

#define LDIR()                do { OutputWrite2Bytes(0xED, 0xB0); reg.BCIsNumber = true; reg.BCIsVariable = false; reg.BCValue = 0; ResetHL(); ResetDE(); } while (0)
#define LDDR()                do { OutputWrite2Bytes(0xED, 0xB8); reg.BCIsNumber = true; reg.BCIsVariable = false; reg.BCValue = 0; ResetHL(); ResetDE(); } while (0)

#define OR_A_A()              do { OutputWriteByte(OP_OR_A_A); } while (0)
#define OR_A_C()              do { OutputWriteByte(OP_OR_A_C); ResetA(); } while (0)
#define XOR_A_A()             do { OutputWriteByte(OP_XOR_A_A); reg.AIsNumber = true; reg.AIsVariable = false; reg.AValue = 0; } while (0)
#define AND_A(val)            do { OutputWriteByte(OP_AND_A); OutputWriteByte(val); ResetA(); } while (0)
#define CP_A(val)             do { OutputWriteByte(OP_CP_A); OutputWriteByte(val); } while (0)
#define CCF()                 do { OutputWriteByte(OP_CCF); } while (0)
#define SCF()                 do { OutputWriteByte(OP_SCF); } while (0)
#define SRL_A()               do { OutputWrite2Bytes(0xCB, 0x3F); ResetA(); } while (0)
#define SUB_A(val)            do { OutputWriteByte(OP_SUB_A); OutputWriteByte(val); ResetA(); } while (0)
#define SUB_A_D()             do { OutputWriteByte(OP_SUB_A_D); ResetA(); } while (0)
#define SUB_A_L()             do { OutputWriteByte(OP_SUB_A_L); ResetA(); } while (0)
#define ADD_A_A()             do { OutputWriteByte(OP_ADD_A_A); ResetA(); } while (0)
#define ADD_A_L()             do { OutputWriteByte(OP_ADD_A_L); ResetA(); } while (0)
#define ADC_A_H()             do { OutputWriteByte(OP_ADC_A_H); ResetA(); } while (0)
#define ADD_A(val)            do { OutputWriteByte(OP_ADD_A); OutputWriteByte(val); ResetA(); } while (0)
#define SBC_A_A()             do { OutputWriteByte(OP_SBC_A_A); ResetA(); } while (0)
#define INC_A()               do { OutputWriteByte(OP_INC_A); ResetA(); } while (0)
#define DEC_A()               do { OutputWriteByte(OP_DEC_A); ResetA(); } while (0)
#define XOR_A(val)            do { OutputWriteByte(OP_XOR_A); OutputWriteByte(val); ResetA(); } while (0)
#define OR_A(val)             do { OutputWriteByte(OP_OR_A); OutputWriteByte(val); ResetA(); } while (0)

#endif
