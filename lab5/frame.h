
// /*Lab5: This header file is not complete. Please finish it with more definition.*/

// #ifndef FRAME_H
// #define FRAME_H

// #include "tree.h"

// extern const int F_wordSize;

// /* declaration for frame */
// typedef struct F_access_ *F_access;
// typedef struct F_accessList_ *F_accessList;
// typedef struct F_frame_ *F_frame;

// struct F_access_
// {
// 	enum
// 	{
// 		inFrame,
// 		inReg
// 	} kind;
// 	union
// 	{
// 		int offset;	   //inFrame
// 		Temp_temp reg; //inReg
// 	} u;
// };

// struct F_accessList_
// {
// 	F_access head;
// 	F_accessList tail;
// };

// struct F_frame_
// {
// 	Temp_label label;
// 	T_stmList stmList;
// 	F_accessList formals;
// 	int size;
// };

// Temp_label F_name(F_frame f);
// F_accessList F_formals(F_frame f);
// F_accessList F_AccessList(F_access head, F_accessList tail);
// F_frame F_newFrame(Temp_label name, U_boolList formals);
// F_access F_allocLocal(F_frame f, bool escape);

// /* declaration for IR */
// Temp_temp F_FP(void);
// T_exp F_Exp(F_access a, T_exp frame);
// T_exp F_externalCall(string s, T_expList args);

// /* declaration for fragments */
// typedef struct F_frag_ *F_frag;
// typedef struct F_fragList_ *F_fragList;

// struct F_frag_
// {
// 	enum
// 	{
// 		F_stringFrag,
// 		F_procFrag
// 	} kind;
// 	union
// 	{
// 		struct
// 		{
// 			Temp_label label;
// 			string str;
// 		} stringg;
// 		struct
// 		{
// 			T_stm body;
// 			F_frame frame;
// 		} proc;
// 	} u;
// };

// struct F_fragList_
// {
// 	F_frag head;
// 	F_fragList tail;
// };

// F_frag F_StringFrag(Temp_label label, string str);
// F_frag F_ProcFrag(T_stm body, F_frame frame);
// F_fragList F_FragList(F_frag head, F_fragList tail);

// #endif

/*Lab5: This header file is not complete. Please finish it with more definition.*/

#ifndef FRAME_H
#define FRAME_H

#include "tree.h"

extern const int F_wordSize;
extern const int F_formalRegNum;
extern const int F_regNum;

/* declaration for frame */
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
	Temp_label label;
	T_stmList stmList;
	F_accessList formals;
	int size;
};

Temp_label F_label(F_frame f);
F_accessList F_formals(F_frame f);
F_accessList F_AccessList(F_access head, F_accessList tail);
F_frame F_newFrame(Temp_label name, U_boolList formals);
F_access F_allocLocal(F_frame f, bool escape);

/* declaration for IR */
Temp_temp F_FP(void);
T_exp F_Exp(F_access a, T_exp frame);
T_exp F_externalCall(string s, T_expList args);

/* declaration for fragments */
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

#endif
