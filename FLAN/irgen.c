#include "irgen.h"

#define push_err(str) do { \
	printf(color_err "Line %u: " str, node->col); \
	abort(); } while(0)

void sym_create(symbol* sym, addr_type atype, addr_t addr, tytree_node* type)
{
	sym->addrtype = atype;
	sym->type = type;
	sym->addr = addr;
}

void syt_create(symbol_table* syt)
{
	syt->depth = 0;
	syt->stack_bottom = 0;
	syt->in_func_block = false;
	varr_create(&(syt->varr), hash_table, 16);
	syt_enter(syt);
}

void syt_destroy(symbol_table* syt)
{
	for (size_t i = 0; i < syt->depth; i++)
	{
		hash_table* cur = varr_get(&(syt->varr), hash_table, i);
		htb_foreach(cur, sym_destroy);
		htb_destroy(cur);
	}
	varr_destroy(&(syt->varr));
}

symbol* syt_find(symbol_table* syt, const char* str)
{
	size_t i = syt->depth - 1;
	while (i-- > 0)
	{
		hash_table* cur = varr_get(&(syt->varr), hash_table, i);
		symbol* find_value = htb_find(cur, str);
		if (find_value != NULL)
			return find_value;
	}
	return NULL;
}

void syt_insert(symbol_table* syt, AST_node* decltree)
{
	hash_table* cur = varr_get(&(syt->varr), hash_table, syt->depth - 1);
	AST_node* type = decltree->children[0];
	AST_node* idinit_head = decltree->children[1];
	long long int offset = 0; //ebp 기준 오프셋

	while (idinit_head->children[0] != NULL)
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
	hash_table* cur = varr_get(&(syt->varr), hash_table, syt->depth - 1);
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
	irg->irs_len = 0;
	irg->data_end = 0;
	irg->mvar_count = 2;

	irg->literal_type[LTT_INT] = tytreend_create(TYTR_INT, 0);
	irg->literal_type[LTT_UINT] = tytreend_create(TYTR_UINT, 0);
	irg->literal_type[LTT_CHAR] = tytreend_create(TYTR_CHAR, 0);
	irg->literal_type[LTT_BOOL] = tytreend_create(TYTR_BOOL, 0);
	irg->literal_type[LTT_FLOAT] = tytreend_create(TYTR_FLOAT, 0);

	irg->literal_type[LTT_STR] = tytreend_create(TYTR_CONST, 0);
	irg->literal_type[LTT_STR]->children[0] = tytreend_create(TYTR_PTROF, 0);
	irg->literal_type[LTT_STR]->children[0]->children[0] = tytreend_create(TYTR_CHAR, 0);
}

void irgen_destroy(irgen* irg)
{
	varr_destroy(&(irg->irs));
	syt_destroy(&(irg->syt));
	htb_destroy(&(irg->str_addr));

	for (size_t i = 0; i < 6; i++)
		tytree_destroy(irg->literal_type[i]);
}

ir* irgen_push(irgen* irg, ir_type type, mvar_code mvcode)
{
	ir* new_ir = varr_get(&(irg->irs), ir, irg->irs_len++);
	new_ir->type = type;
	new_ir->dest = mvcode;
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
	new->op1.asize = SZ_QWORD;

	addr_t* found_addr = htb_find(&(irg->str_addr), node->attr);
	if (found_addr == NULL)
	{
		size_t str_len = strlen(node->attr) + 1;
		size_t str_addr = irg->data_end;
		irg->data_end += str_len;
		addr_t* inserted = htb_insert(&(irg->str_addr), node->attr);
		*inserted = str_addr;

		new->op2.addr = str_addr;
		return irg->literal_type[LTT_STR];
	}
	else
	{
		new->op2.addr = *found_addr;
		return irg->literal_type[LTT_STR];
	}
}

/*
LOAD_CONST QWORD, value, ret
            또는
LOAD_CONST BYTE, value, ret
*/
ir_access_size literal_size[] = { SZ_QWORD, SZ_QWORD, SZ_BYTE, SZ_BYTE, SZ_QWORD, SZ_QWORD };
static tytree_node* emitRval_literal(AST_node* node, irgen* irg, mvar_code mvcode, literal_type_idx idx)
{
	ir* new = irgen_push(irg, IR_LOADCONST, ret);
	new->op1.asize = literal_size[idx];
	switch (idx)
	{
	case LTT_INT:
		new->op2.dec = atoll(node->attr);
		break;
	case LTT_UINT:
		new->op2.dec = atoll(node->attr);
		break;
	case LTT_CHAR:
		new->op2.ch = node->attr[0];
		break;
	case LTT_BOOL:
		if (node->type == AST_TRUE)
			new->op2.boolean = true;
		else
			new->op2.boolean = false;
		break;
	case LTT_FLOAT:
		new->op2.flt = atof(node->attr);
		break;
	}
	return irg->literal_type[idx];
}

static void load_binop_args(AST_node* node, irgen* irg, mvar_code left, mvar_code right, tytree_node** ltype, tytree_node** rtype)
{
	*ltype = emitRval(node->children[0], irg, left);
	*rtype = emitRval(node->children[1], irg, right);
}

static tytree_node* emitRval_arithmetic(AST_node* node, irgen* irg, mvar_code mvcode)
{
	mvar_code left_mv = new_mvar(irg);
	mvar_code right_mv = new_mvar(irg);
	tytree_node *ltype, *rtype;
	load_binop_args(node, irg, left_mv, right_mv, &ltype, &rtype);

	if (!tytree_eq(ltype, rtype))
	{
		push_err("산술 연산자의 피연산자 타입이 일치하지 않습니다.");
	}

	ir_type irt;
	switch (node->type)
	{
		case AST_ADD: irt = IR_ADD; break;
		case AST_SUB: irt = IR_SUB; break;
		case AST_MUL: irt = IR_MUL; break;
		case AST_DIV: irt = IR_DIV; break;
		case AST_MOD: irt = IR_MOD; break;
	}
	ir* new = irgen_push(irg, irt, mvcode);
	new->op1.mvcode = left_mv;
	new->op2.mvcode = right_mv;
	return ltype; //임시로 왼쪽 타입 반환
}

static tytree_node* emitRval_id(AST_node* node, irgen* irg, mvar_code mvcode)
{
	
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
	}
}

static void emitNval_block(AST_node* node, irgen* irg)
{
	syt_enter(&(irg->syt));
	AST_node* stmt_head = node->children[0];
	while (stmt_head != NULL)
	{
		emitNval(stmt_head, irg);
		stmt_head = stmt_head->next;
	}
	syt_exit(&(irg->syt));
}

static void emitNval(AST_node* node, irgen* irg)
{
	switch (node->type)
	{
		case AST_BLOCK: emitNval_block(node, irg);
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
	}
}

void irgen_gen(irgen* irg, AST_node* root)
{
	emitNval(root, irg);
}

