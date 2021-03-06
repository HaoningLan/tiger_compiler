/*Lab5: This header file is not complete. Please finish it with more definition.*/

#ifndef FRAME_H
#define FRAME_H

#include "tree.h"
#include "assem.h"

extern const int F_wordSize;
extern const int F_formalRegNum;
extern const int F_regNum;

//frame
typedef struct F_access_ *F_access;
typedef struct F_accessList_ *F_accessList;
typedef struct F_frame_ *F_frame;

struct F_access_
{
	enum
	{
		inFrame,
		inReg
	} kind;
	union
	{
		int offset;	   //inFrame
		Temp_temp reg; //inReg
	} u;
};

struct F_accessList_
{
	F_access head;
	F_accessList tail;
};

struct F_frame_
{
	F_accessList fmls; // all formals
	T_stm shift; // for view shift
	int size;
	Temp_label name;
};

Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_accessList F_AccessList(F_access head, F_accessList tail);
F_frame F_newFrame(Temp_label name, U_boolList formals);
F_access F_allocLocal(F_frame f, bool escape);

// IR
T_exp F_Exp(F_access acc, T_exp framePtr);
T_exp F_externalCall(string s, T_expList args);
// T_stm F_procEntryExit1(F_frame frame, T_stm stm);

// fragments
typedef struct F_frag_ *F_frag;
typedef struct F_fragList_ *F_fragList;

struct F_frag_
{
	enum
	{
		F_stringFrag,
		F_procFrag
	} kind;
	union
	{
		struct
		{
			Temp_label label;
			string str;
		} stringg;
		struct
		{
			T_stm body;
			F_frame frame;
		} proc;
	} u;
};

struct F_fragList_
{
	F_frag head;
	F_fragList tail;
};

F_frag F_StringFrag(Temp_label label, string str);
F_frag F_ProcFrag(T_stm body, F_frame frame);
F_fragList F_FragList(F_frag head, F_fragList tail);

// Instruction select
Temp_map F_tempMap;
Temp_map F_tempMapInit();

//specialregs
Temp_temp F_FP();
Temp_temp F_SP();
Temp_temp F_RV();
Temp_tempList F_MUL();
Temp_tempList F_DIV();

Temp_tempList F_registers(void);
Temp_tempList F_calleeSaves(void);
Temp_tempList F_callerSaves(void);
Temp_tempList F_paramReg(void);

// proc handler
T_stm F_procEntryExit1(F_frame frame, T_stm stm);
AS_instrList F_procEntryExit2(AS_instrList body);
AS_proc F_procEntryExit3(F_frame frame, AS_instrList body);

Temp_labelList F_preDefineFuncs();

#endif
