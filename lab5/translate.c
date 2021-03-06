#include <stdio.h>
#include "util.h"
#include "table.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"
#include "frame.h"
#include "translate.h"

//LAB5: you can modify anything you want.

//frame
Tr_access Tr_Access(Tr_level l, F_access a)
{
	Tr_access ta = checked_malloc(sizeof(*ta));

	ta->level = l;
	ta->access = a;
	return ta;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
	Tr_accessList a = checked_malloc(sizeof(*a));

	a->head = head;
	a->tail = tail;
	return a;
}

static Tr_level outermost = NULL;
Tr_level Tr_outermost(void)
{
	if (outermost == NULL) outermost = Tr_newLevel(NULL, Temp_newlabel(), NULL);
	return outermost;
}

static Tr_accessList makeFormalAccessList(Tr_level l)
{
	Tr_accessList head = NULL;
	Tr_accessList tail = NULL;
	F_accessList f = F_formals(l->frame)->tail; // ignore the static link

	for(; f; f=f->tail) {
		Tr_access a = Tr_Access(l, f->head);
		if (head == NULL) {
			head = tail = Tr_AccessList(a, NULL);
		} else {
			tail->tail = Tr_AccessList(a, NULL);
			tail = tail->tail;
		}
	}
	return head;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals)
{
	Tr_level l = checked_malloc(sizeof(*l));

	l->parent = parent;
	l->frame = F_newFrame(name, U_BoolList(TRUE, formals));
	l->fmls = makeFormalAccessList(l);
	l->name = name;
	return l;
}

Tr_accessList Tr_formals(Tr_level level)
{
	return level->fmls;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape)
{
	return Tr_Access(level, F_allocLocal(level->frame, escape));
}


// IR
Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail)
{
	Tr_expList l = checked_malloc(sizeof(*l));

	l->head = head;
	l->tail = tail;
	return l;
}

static patchList PatchList(Temp_label *head, patchList tail)
{
	patchList l = checked_malloc(sizeof(*l));
	
	l->head = head;
	l->tail = tail;
	return l;
}

static Tr_exp Tr_Ex(T_exp ex) 
{
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_ex;
	e->u.ex = ex;
	return e;
}

static Tr_exp Tr_Nx(T_stm nx) 
{
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_nx;
	e->u.nx = nx;
	return e;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm) 
{
	Tr_exp e = checked_malloc(sizeof(*e));
	e->kind = Tr_cx;
	e->u.cx.trues = trues;
	e->u.cx.falses = falses;
	e->u.cx.stm = stm;
	return e;
}

void doPatch(patchList tList, Temp_label label)
{
	for(; tList; tList = tList->tail)
		*(tList->head) = label;
}

patchList joinPatch(patchList first, patchList second)
{
	if(!first) return second;
	for(; first->tail; first = first->tail);
	first->tail = second;
	return first;
}

static T_exp unEx(Tr_exp e)
{
	switch (e->kind)
	{
	case Tr_ex:
	    return e->u.ex;
	case Tr_nx:
		return T_Eseq(e->u.nx,T_Const(0));
	case Tr_cx:
	    {
			Temp_temp r = Temp_newtemp();
			Temp_label t = Temp_newlabel(), f = Temp_newlabel();
			doPatch(e->u.cx.trues, t);
			doPatch(e->u.cx.falses, f);
			return T_Eseq(T_Move(T_Temp(r), T_Const(1)),
						  T_Eseq(e->u.cx.stm,
								 T_Eseq(T_Label(f),
										T_Eseq(T_Move(T_Temp(r), T_Const(0)),
											   T_Eseq(T_Label(t),
													  T_Temp(r))))));
		}
	default:
		assert(0);
	}
}

static T_stm unNx(Tr_exp e)
{
	switch (e->kind)
	{
	case Tr_ex:
		return T_Exp(e->u.ex);
	case Tr_nx:
		return e->u.nx;
	case Tr_cx:
	{
		Temp_temp r = Temp_newtemp();
		Temp_label t = Temp_newlabel(), f = Temp_newlabel();
		doPatch(e->u.cx.trues, t);
		doPatch(e->u.cx.falses, f);
		//TODO:check 
		return T_Exp(
			T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				   T_Eseq(e->u.cx.stm,
						  T_Eseq(T_Label(f),
								 T_Eseq(T_Move(T_Temp(r), T_Const(0)),
										T_Eseq(T_Label(t),
											   T_Temp(r)))))));
	}
		default:
			assert(0);
		}
}


static struct Cx unCx(Tr_exp e)
{
	switch (e->kind) {
	case Tr_ex:
	{
		struct Cx cx;
		// TODO:check later
		/* treat CONST(1) and CONST(0) specially */
		if (e->u.ex->kind == T_CONST)
		{
			cx.stm = T_Jump(T_Name(NULL), Temp_LabelList(NULL, NULL));
			if (e->u.ex->u.CONST == 0)
			{
				cx.falses = PatchList(&(cx.stm->u.JUMP.exp->u.NAME),
									  PatchList(&(cx.stm->u.JUMP.jumps->head), NULL)); // check later
				return cx;
			}
			else if (e->u.ex->u.CONST == 1)
			{
				cx.trues = PatchList(&(cx.stm->u.JUMP.exp->u.NAME),
									 PatchList(&(cx.stm->u.JUMP.jumps->head), NULL)); // check later
				return cx;
			}
		}
		cx.stm = T_Cjump(T_eq, e->u.ex, T_Const(1), NULL, NULL);
		cx.trues = PatchList(&(cx.stm->u.CJUMP.true), NULL);
		cx.falses = PatchList(&(cx.stm->u.CJUMP.false), NULL);
		return cx;
	}
	case Tr_nx:
		assert(0);
	case Tr_cx:
		return e->u.cx;
	}
	assert(0);
}

// fragment
static F_fragList fragList = NULL;
void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals)
{
	F_frag f = F_ProcFrag(unNx(body), level->frame);
	fragList = F_FragList(f, fragList);
}


F_fragList Tr_getResult(void)
{
	return fragList;
}

Tr_exp Tr_simpleVar(Tr_access a, Tr_level l)
{
	T_exp fp = T_Temp(F_FP());
	Tr_level ll = l;
	while (ll && ll != a->level) {
		F_access staticLink = F_formals(ll->frame)->head;
		fp = F_Exp(staticLink, fp);
		ll = ll->parent;
	}
	return Tr_Ex(F_Exp(a->access, fp));
}

Tr_exp Tr_fieldVar(Tr_exp e, int offset)
{
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(e), T_Const(offset * F_wordSize))));
}

Tr_exp Tr_subscriptVar(Tr_exp b, Tr_exp i)
{
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(b), T_Binop(T_mul, unEx(i), T_Const(F_wordSize)))));
}

static Temp_label nil = NULL;
Tr_exp Tr_nilExp() //TODO:check later
{
	if (nil == NULL) {
		nil = Temp_newlabel();
		return Tr_Ex(T_Eseq(T_Label(nil), T_Name(nil)));
	} else {
		return Tr_Ex(T_Name(nil));
	}
}

Tr_exp Tr_intExp(int i)
{
	return Tr_Ex(T_Const(i));
}

Tr_exp Tr_stringExp(string s)
{
	Temp_label l = Temp_newlabel();
	F_frag f = F_StringFrag(l, s);
	fragList = F_FragList(f, fragList);
	return Tr_Ex(T_Name(l));
}

Tr_exp Tr_callExp(Temp_label label, Tr_level callee, Tr_level caller, Tr_expList args)
{
	/* get static link */
	T_exp fp = T_Temp(F_FP());
	Tr_level l = caller;
	while (l && l != callee->parent) {
		F_access staticLink = F_formals(l->frame)->head;
		fp = F_Exp(staticLink, fp);
		l = l->parent;
	}
	/* make formal list */
	T_expList head = NULL;
	T_expList tail = NULL;
	Tr_expList arglist = args;
	for (; arglist; arglist=arglist->tail) {
		T_exp t = unEx(arglist->head);
		if (head == NULL) {
			head = tail = T_ExpList(t, NULL);
		} else {
			tail->tail = T_ExpList(t, NULL);
			tail = tail->tail;
		}
	}
	return Tr_Ex(T_Call(T_Name(label), T_ExpList(fp, head)));
}

Tr_exp Tr_binExp(A_oper op, Tr_exp left, Tr_exp right)
{
	T_binOp opp;
	switch (op) {
		case A_plusOp: opp = T_plus; break;
		case A_minusOp: opp = T_minus; break;
		case A_timesOp : opp = T_mul; break;
		case A_divideOp: opp = T_div; break;
	}
	return Tr_Ex(T_Binop(opp, unEx(left), unEx(right)));
}

Tr_exp Tr_relExp(A_oper op, Tr_exp left, Tr_exp right)
{
	T_relOp opp;
	switch (op) {
		case A_eqOp: opp = T_eq; break;
		case A_neqOp: opp = T_ne; break;
		case A_leOp: opp = T_le; break;
		case A_ltOp: opp = T_lt; break;
		case A_geOp: opp = T_ge; break;
		case A_gtOp: opp = T_gt; break;
	}
	T_stm stm = T_Cjump(opp, unEx(left), unEx(right), NULL, NULL);
	patchList trues = PatchList(&(stm->u.CJUMP.true), NULL);
	patchList falses = PatchList(&(stm->u.CJUMP.false), NULL);
	return Tr_Cx(trues, falses, stm);
}

Tr_exp Tr_recordExp(int n, Tr_expList fields)
{
	Temp_temp r = Temp_newtemp();

	T_stm alloc = T_Move(T_Temp(r), F_externalCall("initRecord", T_ExpList(T_Const(n * F_wordSize), NULL)));
	Tr_expList l = fields;
	T_stm seq = T_Move(T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(0))), unEx(l->head));
	l = l->tail;
	for (int i = 1; l; l=l->tail, i++) {
		seq = T_Seq(seq, T_Move(T_Mem(T_Binop(T_plus, T_Temp(r), T_Const(i * F_wordSize))), unEx(l->head))); //TODO:check later
	}
	/* tree struture is different from the tree in the textbook */
	return Tr_Ex(T_Eseq(T_Seq(alloc, seq), T_Temp(r)));
}

Tr_exp Tr_seqExp(Tr_expList exps)
{
	T_exp l = unEx(exps->head);
	exps = exps->tail;
	for (; exps; exps=exps->tail) {
		l = T_Eseq(T_Exp(unEx(exps->head)), l);
	}
	return Tr_Ex(l);
}

Tr_exp Tr_assignExp(Tr_exp lval, Tr_exp v)
{
	return Tr_Nx(T_Move(unEx(lval), unEx(v)));
}

Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee)
{
	
	Temp_label t = Temp_newlabel();
	Temp_label f = Temp_newlabel();
	Temp_label done = Temp_newlabel();
	Temp_temp r = Temp_newtemp();

	struct Cx c = unCx(test);
	doPatch(c.trues, t);
	doPatch(c.falses, f);

	/* the simplest version */
	/* TODO:treat specially when then or else is Cx */
	if (elsee == NULL) {
		return Tr_Nx(T_Seq(c.stm,
						T_Seq(T_Label(t),
							T_Seq(unNx(then), T_Label(f)))));
	} else {
		return Tr_Ex(T_Eseq(T_Seq(c.stm,
								T_Seq(T_Label(t),
									T_Seq(T_Move(T_Temp(r), unEx(then)),
										T_Seq(T_Jump(T_Name(done), Temp_LabelList(done, NULL)),
											T_Seq(T_Label(f),
												T_Seq(T_Move(T_Temp(r), unEx(elsee)),
													T_Seq(T_Jump(T_Name(done), Temp_LabelList(done, NULL)), T_Label(done)))))))) , T_Temp(r)));
	}
}

Tr_exp Tr_whileExp(Tr_exp test, Tr_exp body, Temp_label done)
{
	Temp_label testt = Temp_newlabel();
	Temp_label bodyy = Temp_newlabel();

	struct Cx c = unCx(test);
	doPatch(c.trues, bodyy);
	doPatch(c.falses, done);
	/*
	 * test:
	 * 		if not (condition) goto done
	 * 		body
	 * 		goto test
	 * done:
	 */
	return Tr_Nx(T_Seq(T_Label(testt),
					T_Seq(c.stm,
						T_Seq(T_Label(bodyy),
							T_Seq(unNx(body),
								T_Seq(T_Jump(T_Name(testt), Temp_LabelList(testt, NULL)), T_Label(done)))))));
}

Tr_exp Tr_forExp(Tr_access i, Tr_exp lo, Tr_exp hi, Tr_exp body, Temp_label done)
{
	Temp_label testt = Temp_newlabel();
	Temp_label bodyy = Temp_newlabel();
	T_exp iexp = F_Exp(i->access, T_Temp(F_FP()));
	/*
	 * 		i = lo
	 * test:
	 * 		if i <= hi goto body else done
	 * body:
	 * 		body
	 * 		i = i + 1
	 * 		goto test
	 * done:
	 */
	return Tr_Nx(T_Seq(T_Move(iexp, unEx(lo)),
					T_Seq(T_Label(testt),
						T_Seq(T_Cjump(T_le, iexp, unEx(hi), bodyy, done),
							T_Seq(T_Label(bodyy),
								T_Seq(unNx(body),
									T_Seq(T_Move(iexp, unEx(Tr_binExp(A_plusOp, Tr_Ex(iexp), Tr_Ex(T_Const(1))))),
										T_Seq(T_Jump(T_Name(testt), Temp_LabelList(testt, NULL)), T_Label(done)))))))));
}

Tr_exp Tr_breakExp(Temp_label done)
{
	return Tr_Nx(T_Jump(T_Name(done), Temp_LabelList(done, NULL)));
}

Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init)
{
	return Tr_Ex(F_externalCall("initArray", T_ExpList(unEx(size), T_ExpList(unEx(init), NULL))));
}

Tr_exp Tr_noExp()
{
	return Tr_Ex(T_Const(0)); //TODO:check later
}

void Tr_print(Tr_exp e)
{
	printStmList(stdout, T_StmList(unNx(e), NULL));
}