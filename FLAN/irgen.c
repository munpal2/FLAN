#include "irgen.h"

static void push_err(AST_node* node, const char* str)
{
	printf(color_err "%s(Line %u): %s", node->filename, node->col, str); 
	abort();
}

const char* ir_strty[] = {
	"ADD",   "SUB",     "MUL",       "DIV",      "MOD",
	"NEG",   "INC",     "DEC",       "LOAD",     "STORE",
	"AND",   "OR",      "XOR",       "LSH",      "RSH",
	"NOT",   "CMP",     "GT",        "LT",       "JMP",
	"JZ",    "JNZ",     "CALL",      "RET",      "ALLOC",
	"FREE",  "SYSCALL", "LOADCONST", "MOVE",     "ADDF",
	"SUBF",  "MULF",    "DIVF",      "ITOF",     "FTOI",
	"NEGF",  "GTF",     "LTF",       "MEMCPY"    "ERR"
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
		
		bool is_defined = htb_find(cur, id_node->attr) != NULL;
		if (is_defined)
		{
			printf(color_err "%s(Line %u): 변수 %s가 재정의되었습니다.\n", id_node->filename, id_node->col, id_node->attr);
			abort();
		}
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
	if (type->type == TYTR_CONST || type->type == TYTR_ARROF)
		return tytree_to_irsize(type->children[0]);
	return access_size[type->type];
}

void irgen_create(irgen* irg)
{
	varr_create(&(irg->type_storage), tytree_node*, 256);
	varr_create(&(irg->irs), ir, 100);
	varr_create(irg->section + DATA_SECTION, char, 2048);
	varr_create(irg->section + GLOBAL_SECTION, char, 2048);
	for (size_t i = DATA_SECTION; i <= REG_SECTION; i++)
	{
		varr_create(irg->addr_on + i, qword*, 100);
	}

	htb_create(&(irg->str_addr), addr_t);
	syt_create(&(irg->syt));
	lit_types_init();
	irg->reg_count = 2;
}

void irgen_destroy(irgen* irg)
{
	lit_types_destroy();
	tytree_node** arrayified = irg->type_storage.data;
	for (size_t i = 0; i < irg->type_storage.size; i++)
	{
		tytree_destroy(arrayified[i]);
	}
	varr_destroy(&(irg->type_storage));

	varr_destroy(&(irg->irs));
	for (size_t i = 0; i < irg->section[DATA_SECTION].size; i++)
	{
		char ch = *(char*)varr_get(irg->section + DATA_SECTION, i);
		printf("%hhd(%c) ", ch, ch);
	}

	varr_destroy(irg->section + DATA_SECTION);
	varr_destroy(irg->section + GLOBAL_SECTION);
	for (size_t i = DATA_SECTION; i < REG_SECTION; i++)
	{
		varr_destroy(irg->addr_on + i);
	}

	syt_destroy(&(irg->syt));
	htb_destroy(&(irg->str_addr));
}

static void set_addr(irgen* irg, ir* dest_ir, size_t num, addr_t addr)
{
	dest_ir->args[num].addr = addr;
	section_type idxof_addr_on = ((long long)(addr && 0xff00000000000000LL) >> 56LL);
	if (idxof_addr_on >= CODE_SECTION) //이건 추적 안해도 됨
		return;
	qword** mark_dest = varr_push(irg->addr_on + idxof_addr_on);
	*mark_dest = dest_ir->args + num;
}

ir* irgen_push(irgen* irg, ir_type type, addr_t dest)
{
	ir* new_ir = varr_push(&(irg->irs));
	new_ir->type = type;
	set_addr(irg, new_ir, 2, dest);
	return new_ir;
}

static inline addr_t new_reg(irgen* irg)
{
	return (irg->reg_count++ * 8LL) | ADDR_ON_REG;
}

static void emitNval(AST_node* node, irgen* irg);
static tytree_node* emitRval(AST_node* node, irgen* irg, addr_t addr);
static tytree_node* emitLval(AST_node* node, irgen* irg, addr_t addr);

static tytree_node* emitLval_id(AST_node* node, irgen* irg, addr_t addr)
{
	char* idstr = node->attr;
	symbol* sym = syt_find(&(irg->syt), idstr);
	if (sym == NULL) //있는지 찾기
		push_err(node, "선언되지 않은 변수를 참조하고 있습니다.\n");

	if (sym->addrtype == ADDR_ABS) //절대주소
	{
		ir* loadaddr_ir = irgen_push(irg, IR_LOADCONST, addr);
		loadaddr_ir->args[0].dec = sym->addr;
	}
	else if (sym->addrtype == ADDR_REL) //상대주소
	{
		addr_t offset_addr = new_reg(irg);
		ir* loadoffset_ir = irgen_push(irg, IR_LOADCONST, offset_addr);
		loadoffset_ir->args[0].dec = sym->addr;
		ir* caladdr_ir = irgen_push(irg, IR_ADD, addr);
		set_addr(irg, caladdr_ir, 0, EBP_ADDR);
		set_addr(irg, caladdr_ir, 1, offset_addr);
	}
	return sym->type;
}

static tytree_node* emitLval_ref(AST_node* node, irgen* irg, addr_t addr)
{
	tytree_node* dest_type = emitRval(node->children[0], irg, addr);
	return dest_type->children[0];
}

static tytree_node* emitLval_idx(AST_node* node, irgen* irg, addr_t addr)
{
	addr_t left_addr = new_reg(irg);
	tytree_node* ltype = emitLval(node->children[0], irg, addr);
	addr_t right_addr = new_reg(irg);
	tytree_node* rtype = emitRval(node->children[1], irg, addr);
		
	if (tytree_get_element_type(ltype) == NULL)
		push_err(node, "포인터 또는 배열이 아닙니다. \n");

	addr_t elem_size_addr = new_reg(irg);
	ir* new = irgen_push(irg, IR_LOADCONST, elem_size_addr);
	addr_t elem_size = tytree_sizeof(tytree_get_element_type(ltype));
	new->args[0].dec = elem_size;

	new = irgen_push(irg, IR_MUL, right_addr);
	set_addr(irg, new, 0, right_addr);
	set_addr(irg, new, 1, elem_size_addr);
		
	new = irgen_push(irg, IR_ADD, addr);
	set_addr(irg, new, 0, left_addr);
	set_addr(irg, new, 1, right_addr);
	return tytree_get_element_type(ltype);
}

static tytree_node* emitLval(AST_node* node, irgen* irg, addr_t addr) //ret에 Lvalue를 저장해주시오
{
	switch (node->type)
	{
	case AST_ID:
		return emitLval_id(node, irg, addr);
	case AST_REF:
		return emitLval_ref(node, irg, addr);
	case AST_IDX:
		return emitLval_idx(node, irg, addr);
	default:
		push_err(node, "lvalue값이 필요합니다. \n");
	}
}

static tytree_node* emitRval_str(AST_node* node, irgen* irg, addr_t addr)
{
	ir* new = irgen_push(irg, IR_LOADCONST, addr);

	addr_t* found_addr = htb_find(&(irg->str_addr), node->attr);
	if (found_addr == NULL)
	{
		variable_arr* data_section = irg->section + DATA_SECTION;
		size_t str_len = strlen(node->attr) + 1;
		size_t str_offset = data_section->size;
		addr_t str_address = ADDR_ON_DATA | data_section->size; //string data는 DATA section에 있음
		addr_t* inserted = htb_insert(&(irg->str_addr), node->attr);
		*inserted = str_address;

		/*
		기능 1 : size를 늘리기
		기능 2 : memcpy 할 때 메모리 공간 초과하지 않도록 str_address + str_len이 존재함을 보증하기
		*/
		varr_get(data_section, str_offset + str_len);
		char* start = (char*)data_section->data + str_offset;
		memcpy(start, node->attr, str_len);

		set_addr(irg, new, 0, str_address);
		return literal_type[LTT_STR];
	}
	else
	{
		set_addr(irg, new, 0, *found_addr);
		return literal_type[LTT_STR];
	}
}

/*
LOAD_CONST value, ret
*/
ir_access_size literal_size[] = { SZ_QWORD, SZ_QWORD, SZ_BYTE, SZ_BYTE, SZ_QWORD, SZ_QWORD };
static tytree_node* emitRval_literal(AST_node* node, irgen* irg, addr_t addr, literal_type_idx idx)
{
	ir* new = irgen_push(irg, IR_LOADCONST, addr);
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

static tytree_node* emitRval_binopr_ptrptr(AST_node* node, irgen* irg, addr_t addr,
	addr_t left_addr, tytree_node* ltype,
	addr_t right_addr, tytree_node* rtype)
{
	if (!tytree_eq(ltype, rtype))
		push_err(node, "서로 다른 타입의 포인터는 서로 연산할 수 없습니다.");
	
	ir_type ir_type = IR_ERR;
	switch (node->type)
	{
	case AST_BOR: ir_type = IR_OR; break;
	case AST_BXOR: ir_type = IR_XOR; break;
	case AST_BAND: ir_type = IR_AND; break;
	default:
		push_err(node, "포인터와 포인터 사이에서 정의되지 않은 연산자입니다.");
	}

	ir* new = irgen_push(irg, ir_type, addr);
	set_addr(irg, new, 0, left_addr);
	set_addr(irg, new, 1, right_addr);
	return ltype; //ltype == rtype
}

static tytree_node* emitRval_addsub_ptrint(AST_node* node, irgen* irg, addr_t dest_addr,
	addr_t ptr_addr, tytree_node* ptrtype,
	addr_t int_addr, tytree_node* inttype)
{
	addr_t elem_size_addr = new_reg(irg);
	ir* new = irgen_push(irg, IR_LOADCONST, elem_size_addr);
	addr_t elem_size = tytree_sizeof(tytree_get_element_type(ptrtype));
	new->args[0].dec = elem_size;

	new = irgen_push(irg, IR_MUL, int_addr);
	set_addr(irg, new, 0, int_addr);
	set_addr(irg, new, 1, elem_size_addr);

	new = irgen_push(irg, (node->type == AST_ADD) ? IR_ADD : IR_SUB, dest_addr);
	set_addr(irg, new, 0, ptr_addr);
	set_addr(irg, new, 1, int_addr);
	return ptrtype;
}

static tytree_node* emitRval_binopr_ptrint(AST_node* node, irgen* irg, addr_t dest_addr,
	addr_t ptr_addr, tytree_node* ptrtype,
	addr_t int_addr, tytree_node* inttype)
{
	ir_type ir_type = IR_ERR;
	switch (node->type)
	{
	case AST_ADD:
	case AST_SUB:
		return emitRval_addsub_ptrint(node, irg, dest_addr, ptr_addr, ptrtype, int_addr, inttype);
	case AST_BOR: ir_type = IR_OR; break;
	case AST_BXOR: ir_type = IR_XOR; break;
	case AST_BAND: ir_type = IR_AND; break;
	default:
		push_err(node, "포인터와 정수 사이에서 정의되지 않은 연산자입니다.");
	}

	ir* new = irgen_push(irg, ir_type, dest_addr);
	set_addr(irg, new, 0, ptr_addr);
	set_addr(irg, new, 1, int_addr);
	return ptrtype;
}

static tytree_node* emitRval_binopr_intint(AST_node* node, irgen* irg, addr_t dest_addr,
	addr_t left_addr, tytree_node* ltype,
	addr_t right_addr, tytree_node* rtype)
{
	bool l_is_ptr = tytree_is_ptr(ltype);
	bool r_is_ptr = tytree_is_ptr(rtype);
	if (l_is_ptr && r_is_ptr)
		return emitRval_binopr_ptrptr(node, irg, dest_addr, left_addr, ltype, right_addr, rtype);
	else if (l_is_ptr)
		return emitRval_binopr_ptrint(node, irg, dest_addr, left_addr, ltype, right_addr, rtype);
	else if (r_is_ptr)
		return emitRval_binopr_ptrint(node, irg, dest_addr, right_addr, rtype, left_addr, ltype);
	else
	{
		ir_type ir_type = IR_ERR;
		switch (node->type)
		{
		case AST_ADD: ir_type = IR_ADD; break;
		case AST_SUB: ir_type = IR_SUB; break;
		case AST_MUL: ir_type = IR_MUL; break;
		case AST_DIV: ir_type = IR_DIV; break;
		case AST_MOD: ir_type = IR_MOD; break;
		case AST_BOR: ir_type = IR_OR; break;
		case AST_BXOR: ir_type = IR_XOR; break;
		case AST_BAND: ir_type = IR_AND; break;
		case AST_LSHIFT: ir_type = IR_LSH; break;
		case AST_RSHIFT: ir_type = IR_RSH; break;
		default:
			push_err(node, "정수와 정수 사이에서 정의되지 않은 연산자입니다.\n");
		}

		ir* new = irgen_push(irg, ir_type, dest_addr);
		set_addr(irg, new, 0, left_addr);
		set_addr(irg, new, 1, right_addr);
		return literal_type[LTT_INT]; //int로 업캐스팅
	}
}

static tytree_node* emitRval_binopr_fltflt(AST_node* node, irgen* irg, addr_t dest_addr,
	addr_t left_addr, tytree_node* ltype,
	addr_t right_addr, tytree_node* rtype)
{
	ir_type ir_type = IR_ERR;
	switch (node->type)
	{
	case AST_ADD: ir_type = IR_ADDF; break;
	case AST_SUB: ir_type = IR_SUBF; break;
	case AST_MUL: ir_type = IR_MULF; break;
	case AST_DIV: ir_type = IR_DIVF; break;
	default:
		push_err(node, "실수와 실수 간에서 정의되지 않은 연산자입니다.\n");
	}

	ir* new = irgen_push(irg, ir_type, dest_addr);
	set_addr(irg, new, 0, left_addr);
	set_addr(irg, new, 1, right_addr);
	return literal_type[LTT_FLOAT];
}

static tytree_node* emitRval_binopr(AST_node* node, irgen* irg, addr_t dest_addr,
	addr_t left_addr, tytree_node* ltype,
	addr_t right_addr, tytree_node* rtype)
{
	bool l_is_int = tytree_is_nearint(ltype);
	bool r_is_int = tytree_is_nearint(rtype);
	if (l_is_int && r_is_int)
		return emitRval_binopr_intint(node, irg, dest_addr, left_addr, ltype, right_addr, rtype);
	else if (!l_is_int && !r_is_int)
		return emitRval_binopr_fltflt(node, irg, dest_addr, left_addr, ltype, right_addr, rtype);
	else
		push_err(node, "서로 연산이 가능한 자료형이 아닙니다. 캐스팅을 시도하세요. \n");
}

static tytree_node* emitRval_unaryopr(AST_node* node, irgen* irg, addr_t dest_addr,
	                                  addr_t left_addr, tytree_node* ltype)
{

}

static tytree_node* emitRval_expr(AST_node* node, irgen* irg, addr_t addr)
{
	addr_t left_addr = new_reg(irg);
	tytree_node* ltype = emitRval(node->children[0], irg, left_addr);
	ltype = tytree_get_base_type(ltype);
	if (node->children[1] == NULL) //unary
		return emitRval_unaryopr(node, irg, addr, left_addr, ltype);

	addr_t right_addr = new_reg(irg);
	tytree_node* rtype = emitRval(node->children[1], irg, right_addr);
	rtype = tytree_get_base_type(rtype);
	return emitRval_binopr(node, irg, addr, left_addr, ltype, right_addr, rtype);
}

static tytree_node* emitRval_id(AST_node* node, irgen* irg, addr_t addr)
{
	addr_t addr_of_addr = new_reg(irg);
	tytree_node* type = emitLval_id(node, irg, addr_of_addr);
	ir* final = irgen_push(irg, IR_LOAD, addr);
	final->args[0].dec = tytree_to_irsize(type);
	set_addr(irg, final, 1, addr_of_addr);
	return type;
}

static tytree_node* emitRval_cast(AST_node* node, irgen* irg, addr_t dest_addr)
{
	tytree_node* into_type = from_AST(node->children[0]);
	*(tytree_node**)varr_push(&(irg->type_storage)) = into_type;

	tytree_node* dest_type = emitRval(node->children[1], irg, dest_addr);

	//준int - 준int간 캐스팅 또는 float - float간 캐스팅
	bool into_nearint = tytree_is_nearint(into_type);
	bool dest_nearint = tytree_is_nearint(dest_type);
	bool into_float = tytree_get_base_type(into_type)->type == TYTR_FLOAT;
	bool dest_float = tytree_get_base_type(dest_type)->type == TYTR_FLOAT;
	if ((dest_nearint && into_nearint) || (dest_float && into_float))
	{
		ir* new = irgen_push(irg, IR_MOVE, dest_addr);
		set_addr(irg, new, 0, dest_addr);
		return into_type;
	}

	//(int, uint) - float간 캐스팅
	bool into_strictly_int = tytree_is_int(into_type);
	bool dest_strictly_int = tytree_is_int(dest_type);

	if (dest_strictly_int && into_float)
	{
		ir* new = irgen_push(irg, IR_ITOF, dest_addr);
		set_addr(irg, new, 0, dest_addr);
		return into_type;
	}
	else if (dest_float && into_strictly_int)
	{
		ir* new = irgen_push(irg, IR_FTOI, dest_addr);
		set_addr(irg, new, 0, dest_addr);
		return into_type;
	}

	//불가능한 경우
	push_err(node, "타입 캐스팅이 가능하지 않습니다. \n");
}

static tytree_node* emitRval_getaddr(AST_node* node, irgen* irg, addr_t dest_addr)
{
	tytree_node* dest_type = emitLval(node->children[0], irg, dest_addr);
	tytree_node** ptrof_dest = varr_push(&(irg->type_storage));
	*ptrof_dest = tytreend_create(TYTR_PTROF, 0);
	(*ptrof_dest)->children[0] = tytree_copy(dest_type);
	return *ptrof_dest;
}

static tytree_node* emitRval_pure_assign(AST_node* node, irgen* irg, 
	addr_t dest_addr, tytree_node* dest_type,
	addr_t right_addr, tytree_node* rtype)
{
	ir* new = irgen_push(irg, IR_STORE, dest_addr);
	new->args[0].asize = tytree_to_irsize(dest_type);
	set_addr(irg, new, 1, right_addr);
	return dest_type;
}

static AST_type decomposed_type(AST_type type)
{
	switch (type)
	{
	case AST_ADDX: return AST_ADD;
	case AST_SUBX: return AST_SUB;
	case AST_MULX: return AST_MUL;
	case AST_DIVX: return AST_DIV;
	case AST_MODX: return AST_MUL;
	case AST_ORX: return AST_BOR;
	case AST_ANDX: return AST_BAND;
	case AST_XORX: return AST_BXOR;
	case AST_LSHIFTX: return AST_LSHIFT;
	case AST_RSHIFTX: return AST_RSHIFT;
	}
}

static tytree_node* emitRval_general_assign(AST_node* node, irgen* irg, addr_t dest_addr)
{
	tytree_node* dest_type = emitLval(node->children[0], irg, dest_addr);
	if (node->type != AST_ASSIGN)
	{
		AST_type old_node_type = node->type; //잠깐 속이기
		node->type = decomposed_type(node->type);
		addr_t calculated_addr = new_reg(irg);
		tytree_node* calcultated_type = emitRval(node, irg, calculated_addr);
		node->type = old_node_type;
		return emitRval_pure_assign(node, irg, dest_addr, dest_type, calculated_addr, calcultated_type);
	}
	addr_t right_addr = new_reg(irg);
	tytree_node* rtype = emitRval(node->children[1], irg, right_addr);
	return emitRval_pure_assign(node, irg, dest_addr, dest_type, right_addr, rtype);
}

static tytree_node* emitRval(AST_node* node, irgen* irg, addr_t addr) //ret에 Rvalue를 저장해주시오
{
	switch (node->type)
	{
	case AST_INT: return emitRval_literal(node, irg, addr, LTT_INT);
	case AST_UINT: return emitRval_literal(node, irg, addr, LTT_UINT);
	case AST_CHAR: return emitRval_literal(node, irg, addr, LTT_CHAR);
	case AST_TRUE: case AST_FALSE: return emitRval_literal(node, irg, addr, LTT_BOOL);
	case AST_FLOAT: return emitRval_literal(node, irg, addr, LTT_FLOAT);
	case AST_STR: return emitRval_str(node, irg, addr);
	case AST_ID: return emitRval_id(node, irg, addr);
	case AST_CAST: return emitRval_cast(node, irg, addr);
	case AST_GETADDR: return emitRval_getaddr(node, irg, addr);

	case AST_ASSIGN:  case AST_ADDX:    case AST_SUBX:
	case AST_MULX:    case AST_DIVX:    case AST_MODX:
	case AST_ORX:     case AST_ANDX:    case AST_XORX:
	case AST_LSHIFTX: case AST_RSHIFTX:
		return emitRval_general_assign(node, irg, addr);

	case AST_ADD:    case AST_SUB:  case AST_MUL:
	case AST_DIV:    case AST_MOD:  case AST_BOR:
	case AST_BAND:   case AST_BXOR: case AST_LSHIFT:
	case AST_RSHIFT:
		return emitRval_expr(node, irg, addr);
	default:
		push_err(node, "rvalue값이 필요합니다.");
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
			emitRval(node, irg, new_reg(irg));
	}
	if (node->next != NULL)
		emitNval(node->next, irg);
}

static void addr_addall(variable_arr* tracking_varr, addr_t offset)
{
	qword** arrayified = tracking_varr->data;
	for (size_t i = 0; i < tracking_varr->size; i++)
	{
		qword* cur = arrayified[i];
		cur->addr = (cur->addr & 0x00ffffffffffffff) + offset;
	}
}

static void determine_addr(irgen* irg)
{
	addr_t code_section_size = irg->irs.size * 32LL;
	printf("code_section_size: %lld \n", code_section_size);
	addr_t reg_section_size = irg->reg_count * 8LL;
	printf("reg_section_size: %lld \n", reg_section_size);
	addr_t data_section_size = irg->section[DATA_SECTION].size;
	printf("data_section_size: %lld \n", data_section_size);

	addr_addall(irg->addr_on + REG_SECTION, code_section_size);
	addr_addall(irg->addr_on + DATA_SECTION, code_section_size + reg_section_size);
	addr_addall(irg->addr_on + GLOBAL_SECTION, code_section_size + reg_section_size + data_section_size);
}

void irgen_gen(irgen* irg, AST_node* root)
{
	emitNval(root, irg);
	determine_addr(irg);
}
