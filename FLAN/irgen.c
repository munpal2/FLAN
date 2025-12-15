#include "irgen.h"

#define push_err(str) do { \
	printf(color_err "Line %u: " str, node->col); \
	abort(); } while(0)

const char* ir_strty[] = {
	"ADD",   "SUB",     "MUL",       "DIV",      "MOD",
	"NEG",   "INC",     "DEC",       "LOAD",     "STORE",
	"AND",   "OR",      "XOR",       "LSHF",     "RSHF",
	"NOT",   "CMP",     "GT",        "LT",       "JMP",
	"JZ",    "JNZ",     "CALL",      "RET",      "ALLOC",
	"FREE",  "SYSCALL", "LOADCONST","MOVE",     "ADDF",
	"SUBF",  "MULF",    "DIVF",      "ITOF",     "FTOI",
	"NEGF",  "CMPF"
};

void sym_create(symbol* sym, addr_type atype, addr_t addr, tytree_node* type)
{
	sym->addrtype = atype;
	sym->type = type;
	sym->addr = addr;
}

void syt_create(symbol_table* syt)
{
	syt->stack_bottom = 0;
	syt->in_func_block = false;
	varr_create(&(syt->varr), hash_table, 16);
	syt_enter(syt);
}

void syt_destroy(symbol_table* syt)
{
	for (size_t i = 0; i < syt->varr.size; i++)
	{
		hash_table* cur = varr_get(&(syt->varr), i);
		htb_foreach(cur, sym_destroy);
		htb_destroy(cur);
	}
	varr_destroy(&(syt->varr));
}

symbol* syt_find(symbol_table* syt, const char* str)
{
	size_t i = syt->varr.size; //i-->0 평가에서 1씩 낮아짐.
	while (i-- > 0)
	{
		hash_table* cur = varr_get(&(syt->varr), i);
		symbol* find_value = htb_find(cur, str);
		if (find_value != NULL)
			return find_value;
	}
	return NULL;
}

void syt_insert(symbol_table* syt, AST_node* decltree)
{
	hash_table* cur = varr_back(&(syt->varr));
	AST_node* type = decltree->children[0];
	AST_node* idinit_head = decltree->children[1];
	long long int offset = 0; //ebp 기준 오프셋

	while (idinit_head != NULL)
	{
		AST_node* id_node = idinit_head->children[0];
		tytree_node* type_tree = from_AST(type);
		offset -= tytree_sizeof(type_tree);
		sym_create(htb_insert(cur, id_node->attr), ADDR_REL, offset, type_tree);
		idinit_head = idinit_head->next;
	}
}

void syt_insertf(symbol_table* syt, AST_node* fdecltree)
{
	hash_table* cur = varr_back(&(syt->varr));
	AST_node* param_head = fdecltree->children[1];
	long long int offset = 16;

	while (param_head != NULL)
	{
		tytree_node* type_tree = from_AST(param_head->children[0]);
		sym_create(htb_insert(cur, param_head->children[1]->attr), ADDR_REL, offset, type_tree);
		offset += tytree_sizeof(type_tree);
		param_head = param_head->next;
	}
}

ir_access_size access_size[] = { SZ_QWORD, SZ_QWORD, SZ_BYTE,   SZ_BYTE,  SZ_QWORD,
								 SZ_QWORD, SZ_QWORD, SZ_UNABLE, SZ_UNABLE };
ir_access_size tytree_to_irsize(tytree_node* type)
{
	if (type->type == TYTR_CONST)
		return tytree_to_irsize(type->children[0]);
	return access_size[type->type];
}

void irgen_create(irgen* irg)
{
	varr_create(&(irg->irs), ir, 100);
	htb_create(&(irg->str_addr), addr_t);
	syt_create(&(irg->syt));
	lit_types_init();
	irg->data_end = 0;
	irg->mvar_count = 2;
}

void irgen_destroy(irgen* irg)
{
	lit_types_destroy();
	varr_destroy(&(irg->irs));
	syt_destroy(&(irg->syt));
	htb_destroy(&(irg->str_addr));
}

ir* irgen_push(irgen* irg, ir_type type, addr_t addr)
{
	ir* new_ir = varr_push(&(irg->irs));
	new_ir->type = type;
	new_ir->args[2].addr = addr;
	return new_ir;
}

static inline mvar_code new_mvar(irgen* irg)
{
	return irg->mvar_count++;
}

static void emitNval(AST_node* node, irgen* irg);
static tytree_node* emitRval(AST_node* node, irgen* irg, mvar_code mvcode);
static tytree_node* emitLval(AST_node* node, irgen* irg, mvar_code mvcode);

static tytree_node* emitLval(AST_node* node, irgen* irg, mvar_code mvcode) //ret에 Lvalue를 저장해주시오
{

}

static tytree_node* emitRval_str(AST_node* node, irgen* irg, mvar_code mvcode)
{
	ir* new = irgen_push(irg, IR_LOADCONST, mvcode);

	addr_t* found_addr = htb_find(&(irg->str_addr), node->attr);
	if (found_addr == NULL)
	{
		size_t str_len = strlen(node->attr) + 1;
		size_t str_addr = irg->data_end;
		irg->data_end += str_len;
		addr_t* inserted = htb_insert(&(irg->str_addr), node->attr);
		*inserted = str_addr;

		new->args[0].addr = str_addr;
		return literal_type[LTT_STR];
	}
	else
	{
		new->args[0].addr = *found_addr;
		return literal_type[LTT_STR];
	}
}

/*
LOAD_CONST value, ret
*/
ir_access_size literal_size[] = { SZ_QWORD, SZ_QWORD, SZ_BYTE, SZ_BYTE, SZ_QWORD, SZ_QWORD };
static tytree_node* emitRval_literal(AST_node* node, irgen* irg, mvar_code mvcode, literal_type_idx idx)
{
	ir* new = irgen_push(irg, IR_LOADCONST, mvcode);
	switch (idx)
	{
	case LTT_INT:
		new->args[0].dec = atoll(node->attr);
		break;
	case LTT_UINT:
		new->args[0].dec = atoll(node->attr);
		break;
	case LTT_CHAR:
		new->args[0].dec = (long long int)node->attr[0];
		break;
	case LTT_BOOL:
		if (node->type == AST_TRUE)
			new->args[0].dec = (long long int)true;
		else
			new->args[0].dec = (long long int)false;
		break;
	case LTT_FLOAT:
		new->args[0].flt = atof(node->attr);
		break;
	}
	return literal_type[idx];
}


static inline tytree_node* emitRval_binopr_ptrint(AST_node* node, irgen* irg, mvar_code mvcode,
	mvar_code ptr_mv, tytree_node* ptrtype,
	mvar_code int_mv, tytree_node* inttype)
{
	if (node->type != AST_ADD && node->type != AST_SUB)
		push_err("포인터와 정수 간의 연산은 덧셈과 뺄셈만 가능합니다. \n");

	mvar_code elem_size_mv = new_mvar(irg);
	ir* new = irgen_push(irg, IR_LOADCONST, elem_size_mv);
	addr_t elem_size = tytree_sizeof(tytree_get_element_type(ptrtype));
	new->args[0].asize = SZ_QWORD;
	new->args[1].dec = elem_size;

	new = irgen_push(irg, IR_MUL, int_mv);
	new->args[0].mvcode = int_mv;
	new->args[1].mvcode = elem_size_mv;

	new = irgen_push(irg, (node->type == AST_ADD) ? IR_ADD : IR_SUB, mvcode);
	new->args[0].mvcode = ptr_mv;
	new->args[1].mvcode = int_mv;
	return ptrtype;
}

static inline tytree_node* emitRval_binopr_intint(AST_node* node, irgen* irg, mvar_code mvcode,
	mvar_code left_mv, tytree_node* ltype,
	mvar_code right_mv, tytree_node* rtype)
{
	bool l_is_ptr = tytree_is_ptr(ltype);
	bool r_is_ptr = tytree_is_ptr(rtype);
	if (l_is_ptr && r_is_ptr)
		push_err("포인터 간 연산은 불가능합니다.\n");
	else if (l_is_ptr)
		return emitRval_binopr_ptrint(node, irg, mvcode, left_mv, ltype, right_mv, rtype);
	else if (r_is_ptr)
		return emitRval_binopr_ptrint(node, irg, mvcode, right_mv, rtype, left_mv, ltype);
	else
	{
		ir_type ir_type;
		switch (node->type)
		{
		case AST_ADD: ir_type = IR_ADD; break;
		case AST_SUB: ir_type = IR_SUB; break;
		case AST_MUL: ir_type = IR_MUL; break;
		case AST_DIV: ir_type = IR_DIV; break;
		case AST_MOD: ir_type = IR_MOD; break;
		default:
			push_err("정수와 정수 사이에서 정의되지 않은 연산자입니다.");
		}

		ir* new = irgen_push(irg, ir_type, mvcode);
		new->args[0].mvcode = left_mv;
		new->args[1].mvcode = right_mv;
		return literal_type[LTT_INT]; //int로 업캐스팅
	}
}

static inline tytree_node* emitRval_binopr_fltflt(AST_node* node, irgen* irg, mvar_code mvcode,
	mvar_code left_mv, tytree_node* ltype,
	mvar_code right_mv, tytree_node* rtype)
{
	ir_type ir_type;
	switch (node->type)
	{
	case AST_ADD: ir_type = IR_ADDF; break;
	case AST_SUB: ir_type = IR_SUBF; break;
	case AST_MUL: ir_type = IR_MULF; break;
	case AST_DIV: ir_type = IR_DIVF; break;
	default:
		push_err("실수와 실수 간에서 정의되지 않은 연산자입니다.\n");
	}

	ir* new = irgen_push(irg, ir_type, mvcode);
	new->args[0].mvcode = left_mv;
	new->args[1].mvcode = right_mv;
	return literal_type[LTT_FLOAT]; //int로 업캐스팅
}

static tytree_node* emitRval_binopr(AST_node* node, irgen* irg, mvar_code mvcode)
{
	mvar_code left_mv = new_mvar(irg);
	tytree_node* ltype = emitRval(node->children[0], irg, left_mv);
	mvar_code right_mv = new_mvar(irg);
	tytree_node* rtype = emitRval(node->children[1], irg, right_mv);

	ltype = tytree_get_base_type(ltype);
	rtype = tytree_get_base_type(rtype);

	bool l_is_int = tytree_is_nearint(ltype);
	bool r_is_int = tytree_is_nearint(rtype);
	if (l_is_int && r_is_int)
		return emitRval_binopr_intint(node, irg, mvcode, left_mv, ltype, right_mv, rtype);
	else if (!l_is_int && !r_is_int)
		return emitRval_binopr_fltflt(node, irg, mvcode, left_mv, ltype, right_mv, rtype);
	else
		push_err("서로 연산이 가능한 자료형이 아닙니다. 캐스팅을 시도하세요.\n");
}

static tytree_node* emitRval_id(AST_node* node, irgen* irg, mvar_code mvcode)
{
	char* idstr = node->attr;
	symbol* sym = syt_find(&(irg->syt), idstr);
	if (sym == NULL) //있는지 찾기
		push_err("선언되지 않은 변수를 참조하고 있습니다.\n");
	mvar_code addr_mv = new_mvar(irg);

	if (sym->addrtype == ADDR_ABS) //절대주소
	{
		ir* loadaddr_ir = irgen_push(irg, IR_LOADCONST, addr_mv);
		loadaddr_ir->args[0].dec = sym->addr;
	}
	else if (sym->addrtype == ADDR_REL) //상대주소
	{
		mvar_code offset_mv = new_mvar(irg);
		ir* loadoffset_ir = irgen_push(irg, IR_LOADCONST, offset_mv);
		loadoffset_ir->args[0].dec = sym->addr;
		ir* caladdr_ir = irgen_push(irg, IR_ADD, addr_mv);
		caladdr_ir->args[0].mvcode = EBP_MVCODE;
		caladdr_ir->args[1].mvcode = offset_mv;
	}
	
	ir* final = irgen_push(irg, IR_LOAD, mvcode);
	final->args[0].mvcode = tytree_to_irsize(sym->type);
	final->args[1].mvcode = addr_mv;
	return sym->type;
}

static tytree_node* emitRval(AST_node* node, irgen* irg, mvar_code mvcode) //ret에 Rvalue를 저장해주시오
{
	switch (node->type)
	{
	case AST_INT: return emitRval_literal(node, irg, mvcode, LTT_INT);
	case AST_UINT: return emitRval_literal(node, irg, mvcode, LTT_UINT);
	case AST_CHAR: return emitRval_literal(node, irg, mvcode, LTT_CHAR);
	case AST_TRUE: case AST_FALSE: return emitRval_literal(node, irg, mvcode, LTT_BOOL);
	case AST_FLOAT: return emitRval_literal(node, irg, mvcode, LTT_FLOAT);
	case AST_STR: return emitRval_str(node, irg, mvcode);
	case AST_ID: return emitRval_id(node, irg, mvcode);
	default: return emitRval_binopr(node, irg, mvcode);
	}
}

static void emitNval_block(AST_node* node, irgen* irg)
{
	syt_enter(&(irg->syt));
	AST_node* stmt_head = node->children[0];
	if (stmt_head != NULL)
		emitNval(stmt_head, irg);
	syt_exit(&(irg->syt));
}

static void emitNval(AST_node* node, irgen* irg)
{
	switch (node->type)
	{
		case AST_BLOCK:
		{
			emitNval_block(node, irg);
			break;
		}
		case AST_DECL:
		{
			syt_insert(&(irg->syt), node);
			break;
		}
		case AST_FDECL:
		{
			syt_insertf(&(irg->syt), node);
			emitNval(node->children[3], irg);
			break;
		}
		default:
			emitRval(node, irg, new_mvar(irg));
	}
	if (node->next != NULL)
		emitNval(node->next, irg);
}

void irgen_gen(irgen* irg, AST_node* root)
{
	emitNval(root, irg);
}

