#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/

//varibales
const int F_wordSize = 8;
const int F_formalRegNum = 6;
const int F_regNum = 16;

//frame

F_accessList F_AccessList(F_access head, F_accessList tail)
{
	F_accessList a = checked_malloc(sizeof(*a));

	a->head = head;
	a->tail = tail;
	return a;
}

static F_access InFrame(int offset)
{
	F_access a = checked_malloc(sizeof(*a));

	a->kind = inFrame;
	a->u.offset = offset;

	return a;
}

static F_access InReg(Temp_temp reg)
{
	F_access a = checked_malloc(sizeof(*a));

	a->kind = inReg;
	a->u.reg = reg;

	return a;
}

static F_accessList makeFormalAccessList(F_frame f,U_boolList formals)
{
	U_boolList boolList = formals;
	F_accessList head = NULL;
	F_accessList tail = NULL;
	for (; boolList; boolList=boolList->tail) {
		F_access a = F_allocLocal(f, boolList->head);
		if (head == NULL) {
			head = tail = F_AccessList(a, NULL);
		} else {
			tail->tail = F_AccessList(a, NULL);
			tail = tail->tail;
		}
	}
	return head;
}

F_frame F_newFrame(Temp_label label, U_boolList formals)
{
	F_frame f = checked_malloc(sizeof(*f));
	f->label = label;
	f->formals = makeFormalAccessList(f,formals);
	f->size = 0;
	return f;
}

Temp_label F_label(F_frame f)
{
	return f->label;
}

F_accessList F_formals(F_frame f)
{
	return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape)
{
	f->size += F_wordSize;
	if (escape)
	{
		return InFrame(-1 * f->size);
	}
	else
	{
		return InReg(Temp_newtemp());
	}
}

//IR
static Temp_temp fp = NULL;
Temp_temp F_FP(void)
{
	if (fp == NULL)
	{
		fp = Temp_newtemp();
	}
	return fp;
}

T_exp F_Exp(F_access a, T_exp frame)
{
	if (a->kind == inFrame)
	{
		return T_Mem(T_Binop(T_plus, frame, T_Const(a->u.offset)));
	}
	else{
		return T_Temp(a->u.reg);
	}
}

T_exp F_externalCall(string s, T_expList args)
{
	return T_Call(T_Name(Temp_namedlabel(s)), args);
}

// fragment

F_frag F_StringFrag(Temp_label label, string str)
{
	F_frag f = checked_malloc(sizeof(*f));
	f->kind = F_stringFrag;
	f->u.stringg.label = label;
	f->u.stringg.str = str;
	return f;
}

F_frag F_ProcFrag(T_stm body, F_frame frame)
{
	F_frag f = checked_malloc(sizeof(*f));
	f->kind = F_procFrag;
	f->u.proc.body = body;
	f->u.proc.frame = frame;
	return f;
}

F_fragList F_FragList(F_frag head, F_fragList tail)
{
	F_fragList fl = checked_malloc(sizeof(*fl));
	fl->head = head;
	fl->tail = tail;
	return fl;
}
