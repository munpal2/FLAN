#include "parser.h"

#define push_err(str) do { \
	printf(color(220, 110, 0) "Line %u: " str, psr->lookahead->col); \
	abort(); } while(0)

#define push_warn(str) printf(color(220, 220, 0) "Line %u: " #str "\n", psr->lookahead->col)

#define FIRST(smt) const token_type smt##_FIRST[]
#define FOLLOW(smt) const token_type smt##_FOLLOW[]
#define LEN(smt) (sizeof(smt) / sizeof(smt[0]))

#define in_FIRST(target, smt) in_impl((target), smt##_FIRST, LEN(smt##_FIRST))
#define in_FOLLOW(target, smt) in_impl((target), smt##_FOLLOW, LEN(smt##_FOLLOW))

const char* AST_strty[] = {"AST_CONJ",  "AST_DECL",    "AST_CONST",   "AST_PTROF",   "AST_ASSIGN",
                           "AST_ADD",   "AST_SUB",     "AST_MUL",     "AST_DIV",     "AST_NEG",
	                       "AST_REF",   "AST_GETADDR", "AST_ID",      "AST_STR",     "AST_INT",
	                       "AST_FLOAT", "AST_UINT",    "AST_TYPE",    "AST_CAST",    "AST_ARR",
	                       "AST_FWINC", "AST_FWDEC",   "AST_BWINC",   "AST_BWDEC",   "AST_DOT",
                           "AST_ARROW", "AST_TRUE",    "AST_FALSE",   "AST_NOT",     "AST_BNOT", 
                           "AST_MOD",   "AST_LSHIFT",  "AST_RSHIFT",  "AST_LT",      "AST_GT",
                           "AST_LTE",   "AST_GTE",     "AST_EQ",      "AST_NEQ",     "AST_BAND",
	                       "AST_BOR",   "AST_BXOR",    "AST_OR",      "AST_AND",     "AST_ADDI",
	                       "AST_SUBI",  "AST_MULI",    "AST_DIVI",    "AST_MODI",    "AST_ANDI",
	                       "AST_ORI",   "AST_XORI",    "AST_LSHIFTI", "AST_RSHIFTI", "AST_WHILE",
                           "AST_IF",    "AST_FDECL",   "AST_BLOCK",   "AST_PARAM",   "AST_RETURN",
                           "AST_IDX",   "AST_FUNC",     "AST_CALL",    "AST_FOR",     "AST_CHAR",
	                       "AST_IDINIT",  "AST_INITEXPR"};

AST_node* AST_node_create(AST_type type, const char* str, unsigned int col)
{
	AST_node* ret = calloc(sizeof(AST_node), 1);
	if (ret == NULL)
		return NULL;
	ret->col = col;
	ret->type = type;
	if (str != NULL)
		ret->attr = _strdup(str);
	return ret;
}

#define node_create_q(type, str) AST_node_create(type, str, psr->lookahead->col)

void AST_node_destroy(AST_node * dest)
{
	free(dest->attr);
	for (size_t i = 0; i < 4; i++)
	{
		if (dest->children[i] != NULL)
			AST_node_destroy(dest->children[i]);
	}
	free(dest);
}

void psr_init(parser* psr, variable_arr* processed_tokens)
{
	psr->idx = 0;
	psr->lookahead = varr_get(processed_tokens, token, 0);
	psr->tokens = processed_tokens;
	psr->root = NULL;
}

void psr_next(parser* psr)
{
	psr->lookahead = varr_get(psr->tokens, token, ++psr->idx);
}

void psr_expect(parser* psr, token_type type)
{
	if (psr->lookahead->type == type)
	{
		psr_next(psr);
		return;
	}
	printf(color(220, 110, 0) "Line %u: %s가 필요하지만 대신 %s가 있습니다.", psr->lookahead->col, token_strty[type], token_strty[psr->lookahead->type]);
	abort();
}

static unsigned int TK_to_AST(token_type TK_type)
{
	switch (TK_type)
	{
		case TK_INT: return AST_INT;
		case TK_ID: return AST_ID;
		case TK_UINT: return AST_UINT;
		case TK_FLOAT: return AST_FLOAT;
		case TK_STR: return AST_STR;
		case TK_FALSE: return AST_FALSE;
		case TK_TRUE: return AST_TRUE;
		case TK_CHAR: return AST_CHAR;
		default:
		{
			puts(color(220, 0, 0) "함수 쓰기전에 생각했나요??");
			printf("방금 TK_to_AST에 %s가 전달되었지만 TK_to_AST는 %s에 대한 변환을 지원하지 않습니다.", token_strty[TK_type], token_strty[TK_type]);
			abort();
		}
	}
}

static bool in_impl(AST_type target, AST_type* set, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (target == set[i])
			return true;
	}
	return false;
}

void psr_destroy(parser* psr)
{
	if (psr->root != NULL)
		AST_node_destroy(psr->root);
}

/* static 함수들의 전방선언 */
static inline AST_node* parse_stmts(parser* psr);
static inline AST_node* parse_exprs(parser* psr);
static AST_node* parse_type_expr(parser* psr);
static AST_node* parse_stmt(parser* psr);
static AST_node* parse_expr(parser* psr);
static AST_node* parse_init_expr(parser* psr);

/* FIRST(expr)은 먼저 선언 */
FIRST(expr) = { TK_ID,   TK_STR,   TK_FLOAT, TK_UINT,       TK_INT,
				TK_TRUE, TK_FALSE, TK_MINUS, TK_OPEN_PAREN, TK_INC,
				TK_DEC,  TK_NOT,   TK_BNOT,  TK_MUL,        TK_BAND };

/*
left_conj -> basic separator left_conj
             basic
*/
static AST_node* parse_left_conj(parser* psr, AST_node* (*parse_basic)(parser*), token_type separator)
{
	AST_node* basic_node = parse_basic(psr);
	if (psr->lookahead->type == separator)
	{
		psr_next(psr);
		AST_node* conj_node = node_create_q(AST_CONJ, NULL);
		conj_node->children[0] = basic_node;
		conj_node->children[1] = parse_left_conj(psr, parse_basic, separator);
		return conj_node;
	}
	return basic_node;
}

static inline AST_node* parse_type_exprs(parser* psr)
{
	return parse_left_conj(psr, parse_type_expr, TK_COMMA);
}

static AST_node* parse_const_texpr(parser* psr)
{
	psr_next(psr);
	AST_node* ret = node_create_q(AST_CONST, NULL);
	ret->children[0] = parse_type_expr(psr);
	return ret;
}

static AST_node* parse_ptr_texpr(parser* psr)
{
	psr_next(psr);
	if (psr->lookahead->type != TK_OF)
		push_err("키워드 \'ptr\'뒤에는 키워드 \'of\'가 와야 합니다.");
	psr_next(psr);
	AST_node* ret = node_create_q(AST_PTROF, NULL);
	ret->children[0] = parse_type_expr(psr);
	return ret;
}

static AST_node* parse_arr_texpr(parser* psr)
{
	psr_next(psr);
	AST_node* ret = node_create_q(AST_ARR, NULL);
	psr_expect(psr, TK_OPEN_PAREN);
	if (psr->lookahead->type != TK_UINT)
		push_err("오직 uint리터럴만 배열의 크기로 사용될 수 있습니다.");

	ret->children[1] = node_create_q(AST_UINT, psr->lookahead->attr); //lookahead는 TK_UINT여야 함
	psr_next(psr);
	psr_expect(psr, TK_CLOSE_PAREN);
	psr_expect(psr, TK_OF);
	ret->children[0] = parse_type_expr(psr);
	return ret;
}

static AST_node* parse_func_texpr(parser* psr)
{
	psr_next(psr);
	AST_node* ret = node_create_q(AST_FUNC, NULL);
	psr_expect(psr, TK_OPEN_PAREN);
	ret->children[0] = parse_type_exprs(psr);
	psr_expect(psr, TK_CLOSE_PAREN);
	psr_expect(psr, TK_COLON);
	ret->children[1] = parse_type_expr(psr);
	return ret;
}

/*
type_expr -> type
			 const type_expr
			 ptr of type_expr
			 arr(uint) of type_expr
			 func(type_exprs): type_expr
*/
FIRST(type_expr) = { TK_TYPE, TK_CONST, TK_PTR, TK_ARR, TK_FUNC };
FOLLOW(type_expr) = { TK_CLOSE_PAREN };

static AST_node* parse_type_expr(parser* psr)
{
	switch (psr->lookahead->type)
	{
	case TK_TYPE:
	{
		AST_node* ret = node_create_q(AST_TYPE, psr->lookahead->attr);
		psr_next(psr);
		return ret;
	}
	case TK_ARR:
		return parse_arr_texpr(psr);
	case TK_PTR:
		return parse_ptr_texpr(psr);
	case TK_CONST:
		return parse_const_texpr(psr);
	case TK_FUNC:
		return parse_func_texpr(psr);
	default:
		push_err("제대로 된 타입 식이 아닙니다.");
	}
}

/*
element -> id
           str
		   float
		   uint
		   int
		   true
		   false
		   -float
		   -int
		   (expr)
*/
FIRST(element) = {TK_ID,    TK_STR, TK_FLOAT, TK_UINT, TK_INT, TK_TRUE, 
                  TK_FALSE, TK_MINUS};
FOLLOW(element) = {TK_DEC, TK_INT, TK_DOT, TK_ARROW};

static AST_node* parse_element(parser* psr)
{
	switch (psr->lookahead->type)
	{
		case TK_ID: case TK_STR: case TK_FLOAT: case TK_UINT: case TK_INT: case TK_TRUE: case TK_FALSE: case TK_CHAR:
		{
			AST_node* ret = node_create_q(TK_to_AST(psr->lookahead->type), psr->lookahead->attr);
			psr_next(psr);
			return ret;
		}
		case TK_MINUS:
		{
			psr_next(psr);
			if (psr->lookahead->type == TK_INT || psr->lookahead->type == TK_FLOAT)
			{
				str_builder tmp;
				str_builder_create(&tmp);
				str_builder_add(&tmp, '-');
				str_builder_add_str(&tmp, psr->lookahead->attr);
				AST_node* ret = node_create_q(TK_to_AST(psr->lookahead->type), str_builder_pop(&tmp));
				str_builder_destroy(&tmp);
				psr_next(psr);
				return ret;
			}
			else if (psr->lookahead->type == TK_UINT || psr->lookahead->type == TK_STR)
				push_err("uint리터럴이나 문자열 앞에는 -가 올 수 없습니다.");
			else goto handle_err;
		}
		case TK_OPEN_PAREN:
		{
			psr_next(psr);
			AST_node* ret = parse_expr(psr);
			psr_expect(psr, TK_CLOSE_PAREN);
			return ret;
		}
		default:
			goto handle_err;
	}
	
handle_err:
	push_err("리터럴이나 변수가 있어야 합니다.");
}

/*
postfix_chain -> .id postfix_chain
                 ->id postfix_chain
				 ++ postfix_chain
				 -- postfix_chain
				 [expr] postfix_chain
				 (exprs) postfix_chain
				 () postfix_chain
				 epsilon
*/
FIRST(postfix_chain) = { TK_DOT, TK_INC, TK_DEC, TK_ARROW, TK_OPEN_PAREN,
                         TK_OPEN_BRACKET};

static AST_node* parse_postfix_chain(parser* psr, AST_node* base) //postfix_chain이 붙는 element 
{
	while (true)
	{
		switch (psr->lookahead->type)
		{
		case TK_INC: case TK_DEC:
		{
			AST_node* new_node = node_create_q(psr->lookahead->type == TK_INC ? AST_BWINC : AST_BWDEC, NULL);
			psr_next(psr);
			new_node->children[0] = base;
			base = new_node;
			break;
		}
		case TK_DOT: case TK_ARROW:
		{
			AST_node* new_node = node_create_q(psr->lookahead->type == TK_DOT ? AST_DOT : AST_ARROW, NULL);
			psr_next(psr);
			new_node->children[0] = base;
			new_node->children[1] = node_create_q(AST_ID, psr->lookahead->attr);
			psr_expect(psr, TK_ID);
			base = new_node;
			break;
		}
		case TK_OPEN_BRACKET:
		{
			AST_node* new_node = node_create_q(AST_IDX, NULL);
			psr_next(psr);
			new_node->children[0] = base;
			new_node->children[1] = parse_expr(psr);
			psr_expect(psr, TK_CLOSE_BRACKET);
			base = new_node;
			break;
		}
		case TK_OPEN_PAREN:
		{
			AST_node* new_node = node_create_q(AST_CALL, NULL);
			psr_next(psr);
			new_node->children[0] = base;
			if (in_FIRST(psr->lookahead->type, expr))
				new_node->children[1] = parse_exprs(psr);
			psr_expect(psr, TK_CLOSE_PAREN);
			base = new_node;
			break;
		}
		default:
			return base;
		}
	}
}

/*
primary -> element
		   element postfix_chain
		   (type_expr)element ex) (const ptr of const uint)a
*/
FIRST(primary) = { TK_ID,   TK_STR,   TK_FLOAT, TK_UINT, TK_INT, 
                   TK_TRUE, TK_FALSE, TK_MINUS, TK_OPEN_PAREN };

static AST_node* parse_primary(parser* psr)
{	
	AST_node* ret;
	if (psr->lookahead->type == TK_OPEN_PAREN)
	{
		psr_next(psr);
		ret = node_create_q(AST_CAST, NULL);
		ret->children[0] = parse_type_expr(psr);
		psr_expect(psr, TK_CLOSE_PAREN);
		ret->children[1] = parse_element(psr);
		return ret;
	}
	
	AST_node* element_node = parse_element(psr);
	if (in_FIRST(psr->lookahead->type, postfix_chain))
		return parse_postfix_chain(psr, element_node);
	else
		return element_node;
}

/*
prefix_chain -> ++ prefix_chain
				-- prefix_chain
				- prefix_chain
				! prefix_chain
				~ prefix_chain
				* prefix_chain
				& prefix_chain
				epsilon

unary -> prefix_chain primary
		 primary
*/
FIRST(prefix_chain) = { TK_ID,   TK_STR,   TK_FLOAT, TK_UINT,       TK_INT,
                        TK_TRUE, TK_FALSE, TK_MINUS, TK_OPEN_PAREN, TK_INC,
	                    TK_DEC,  TK_NOT,   TK_BNOT,  TK_MUL,        TK_BAND };

static AST_node* parse_prefix_chain(parser* psr)
{
	AST_node* ret;
	switch (psr->lookahead->type)
	{
		case TK_INC:
			ret = node_create_q(AST_FWINC, NULL);
			break;
		case TK_DEC:
			ret = node_create_q(AST_FWDEC, NULL);
			break;
		case TK_MINUS:
			ret = node_create_q(AST_NEG, NULL);
			break;
		case TK_NOT:
			ret = node_create_q(AST_NOT, NULL);
			break;
		case TK_BNOT:
			ret = node_create_q(AST_BNOT, NULL);
			break;
		case TK_MUL:
			ret = node_create_q(AST_REF, NULL);
			break;
		case TK_BAND:
			ret = node_create_q(AST_GETADDR, NULL);
			break;
		default:
			return parse_primary(psr); //epsilon -> 그냥 primary를 파싱하면 됨
	}
	psr_next(psr);
	ret->children[0] = parse_prefix_chain(psr);
	return ret;
}

token_type operators[10][4] = { {TK_MUL,    TK_DIV,     TK_MOD}, 
	                              {TK_PLUS,   TK_MINUS},
	                              {TK_LSHIFT, TK_RSHIFT},
	                              {TK_LT,     TK_GT,      TK_LTE,  TK_GTE},
	                              {TK_EQ,     TK_NEQ},
	                              {TK_BAND},
	                              {TK_BXOR},
	                              {TK_BOR},
	                              {TK_AND},
	                              {TK_OR} };
size_t operators_len[10] = {3, 2, 2, 4, 2, 1, 1, 1, 1, 1};
AST_type operators_AST[10][4] = { {AST_MUL,    AST_DIV,     AST_MOD},
	                                  {AST_ADD,    AST_SUB}, 
	                                  {AST_LSHIFT, AST_RSHIFT}, 
	                                  {AST_LT,     AST_GT,      AST_LTE,  AST_GTE},
	                                  {AST_EQ,     AST_NEQ},
	                                  {AST_BAND},
	                                  {AST_BXOR}, 
	                                  {AST_BOR},
	                                  {AST_AND},
	                                  {AST_OR} };

static AST_node* parse_left_assoc(parser* psr, int depth)
{
	if (depth < 0)
		return parse_prefix_chain(psr);	

	AST_node* base = parse_left_assoc(psr, depth - 1);

	while (true)
	{
		size_t idx;
		for (idx = 0; idx < operators_len[depth]; idx++)
		{
			if (operators[depth][idx] == psr->lookahead->type)
			{
				psr_next(psr);
				break;
			}
		}

		if (idx != operators_len[depth])
		{
			AST_node* op_node = node_create_q(operators_AST[depth][idx], NULL);
			op_node->children[0] = base;
			op_node->children[1] = parse_left_assoc(psr, depth - 1);
			base = op_node;
		}
		else
			return base;
	}
}

token_type assigns[11] = {TK_ASSIGN,  TK_ANDEQ, TK_OREQ,  TK_XOREQ, TK_PLUSEQ,
                            TK_MINUSEQ, TK_MULEQ, TK_DIVEQ, TK_MODEQ, TK_SHLEQ,
                            TK_SHREQ};
AST_type assigns_AST[11] = {AST_ASSIGN, AST_ANDI, AST_ORI,  AST_XORI, AST_ADDI,
                                AST_SUBI,   AST_MULI, AST_DIVI, AST_MODI, AST_LSHIFTI, 
                                AST_RSHIFTI};

/*
expr -> lassoc = lassoc
		lassoc += lassoc
				.
				.
				.
		lassoc
*/

static AST_node* parse_expr(parser* psr)
{
	AST_node* left_node = parse_left_assoc(psr, 9);
	size_t idx;
	for (idx = 0; idx < 11; idx++)
	{
		if (assigns[idx] == psr->lookahead->type)
		{
			psr_next(psr);
			break;
		}
	}

	if (idx == 11)
		return left_node;

	AST_node* op_node = node_create_q(assigns_AST[idx], NULL);
	op_node->children[0] = left_node;
	op_node->children[1] = parse_expr(psr);
	return op_node;
}

/*
exprs -> expr, exprs
		 expr
*/
static inline AST_node* parse_exprs(parser* psr)
{
	return parse_left_conj(psr, parse_expr, TK_COMMA);
}

/*
stmt -> { stmts }
*/
static AST_node* parse_block_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* new_node = node_create_q(AST_BLOCK, NULL);
	new_node->children[0] = parse_stmts(psr);
	psr_expect(psr, TK_CLOSE_BRACE);
	return new_node;
}

static AST_node* parse_while_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* new_node = node_create_q(AST_WHILE, NULL);
	psr_expect(psr, TK_OPEN_PAREN);
	new_node->children[0] = parse_expr(psr);
	psr_expect(psr, TK_CLOSE_PAREN);
	new_node->children[1] = parse_stmt(psr);
	return new_node;
}

static AST_node* parse_if_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* new_node = node_create_q(AST_IF, NULL);
	psr_expect(psr, TK_OPEN_PAREN);
	new_node->children[0] = parse_expr(psr);
	psr_expect(psr, TK_CLOSE_PAREN);
	new_node->children[1] = parse_stmt(psr);
	if (psr->lookahead->type == TK_ELSE)
	{
		psr_next(psr);
		new_node->children[2] = parse_stmt(psr);
	}
	return new_node;
}

/*
init_expr_elem -> init_expr
                  expr
*/
static AST_node* parse_init_expr_elem(parser* psr)
{
	if (psr->lookahead->type == TK_OPEN_BRACE)
		return parse_init_expr(psr);
	return parse_expr(psr);
}

/*
init_expr_elems -> init_expr_elem, init_expr_elems 
				   init_expr_elem
*/
static inline AST_node* parse_init_expr_elems(parser* psr)
{
	return parse_left_conj(psr, parse_init_expr_elem, TK_COMMA);
}

/*
init_expr -> { init_expr_elems }
*/
static AST_node* parse_init_expr(parser* psr)
{
	psr_next(psr);
	AST_node* ret = node_create_q(AST_INITEXPR, NULL);
	ret->children[0] = parse_init_expr_elems(psr);
	psr_expect(psr, TK_CLOSE_BRACE);
	return ret;
}

/*
id_init -> id = init_expr
           id = expr
           id
*/
static AST_node* parse_id_init(parser* psr)
{
	AST_node* ret = node_create_q(AST_IDINIT, NULL);
	ret->children[0] = node_create_q(AST_ID, psr->lookahead->attr);
	psr_expect(psr, TK_ID);
	if (psr->lookahead->type == TK_ASSIGN)
	{
		psr_next(psr);
		if (psr->lookahead->type == TK_OPEN_BRACE)
			ret->children[1] = parse_init_expr(psr);
		else 
			ret->children[1] = parse_expr(psr);
	}
	return ret;
}

/*
id_list -> id_init, id_list
           id_init
*/
static inline AST_node* parse_id_list(parser* psr)
{
	return parse_left_conj(psr, parse_id_init, TK_COMMA);
}

/*
decl_stmt -> decl type_expr id_list
*/
static AST_node* parse_decl_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* new_node = node_create_q(AST_DECL, NULL);
	new_node->children[0] = parse_type_expr(psr);
	new_node->children[1] = parse_id_list(psr);
	return new_node;
}

/*
param -> type_expr id
*/
static AST_node* parse_param(parser* psr)
{
	AST_node* ret = node_create_q(AST_PARAM, NULL);
	ret->children[0] = parse_type_expr(psr);
	ret->children[1] = node_create_q(AST_ID, psr->lookahead->attr);
	psr_expect(psr, TK_ID);
	return ret;
}

/*
params -> param, params
          param
*/
FIRST(params) = { TK_TYPE, TK_CONST, TK_PTR, TK_ARR, TK_FUNC };
static inline AST_node* parse_params(parser* psr)
{
	return parse_left_conj(psr, parse_param, TK_COMMA);
}

/*
fdecl_stmt -> func id(params): type_expr stmt
              func id(): type_expr stmt
			  func id(params) stmt
			  func id() stmt
*/
static AST_node* parse_fdecl_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* new_node = node_create_q(AST_FDECL, NULL);
	new_node->children[0] = node_create_q(AST_ID, psr->lookahead->attr);
	psr_expect(psr, TK_ID);

	psr_expect(psr, TK_OPEN_PAREN);
	if (in_FIRST(psr->lookahead->type, params))
		new_node->children[1] = parse_params(psr);
	psr_expect(psr, TK_CLOSE_PAREN);

	if (psr->lookahead->type == TK_COLON)
	{
		psr_next(psr);
		new_node->children[2] = parse_type_expr(psr);
	}
	new_node->children[3] = parse_stmt(psr);
	return new_node;
}

static AST_node* parse_return_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* return_node = node_create_q(AST_RETURN, NULL);
	return_node->children[0] = parse_expr(psr);
	return return_node;
}

static AST_node* parse_for_stmt(parser* psr)
{
	psr_next(psr);
	AST_node* ret = node_create_q(AST_FOR, NULL);
	psr_expect(psr, TK_OPEN_PAREN);
	ret->children[0] = parse_decl_stmt(psr);
	psr_expect(psr, TK_SEMICOLON);
	ret->children[1] = parse_expr(psr);
	psr_expect(psr, TK_SEMICOLON);
	ret->children[2] = parse_exprs(psr);
	psr_expect(psr, TK_CLOSE_PAREN);
	ret->children[3] = parse_stmt(psr);
	return ret;
}

/*
stmt -> { stmts }
        while (expr) stmt
		for (decl_stmt; expr; exprs) stmt
	    if (expr) stmt 
	    if (expr) stmt else stmt
		decl_stmt
		fdecl_stmt
	    return expr
		expr
	    epsilon
*/
FIRST(stmt) = { TK_ID,     TK_STR,   TK_FLOAT, TK_UINT,       TK_INT,
				TK_TRUE,   TK_FALSE, TK_MINUS, TK_OPEN_PAREN, TK_INC,
				TK_DEC,    TK_NOT,   TK_BNOT,  TK_MUL,        TK_BAND,
                TK_WHILE,  TK_FOR,   TK_IF,    TK_OPEN_BRACE, TK_FUNC,
                TK_DECL,   TK_RETURN};
static AST_node* parse_stmt(parser* psr)
{
	if (in_FIRST(psr->lookahead->type, expr))
		return parse_expr(psr);
	switch (psr->lookahead->type)
	{
		case TK_OPEN_BRACE:
			return parse_block_stmt(psr);
		case TK_WHILE:
			return parse_while_stmt(psr);
		case TK_IF:
			return parse_if_stmt(psr);
		case TK_DECL:
			return parse_decl_stmt(psr);
		case TK_FUNC:
			return parse_fdecl_stmt(psr);
		case TK_RETURN:
			return parse_return_stmt(psr);
		case TK_FOR:
			return parse_for_stmt(psr);
		default:
			return NULL;
	}
}

/*
stmts -> stmt; stmts
         stmt
*/
static inline AST_node* parse_stmts(parser* psr)
{
	AST_node* ret = parse_left_conj(psr, parse_stmt, TK_SEMICOLON);
	if (in_FIRST(psr->lookahead->type, stmt))
		push_warn(";이 필요합니다.");
	return ret;
}

AST_node* psr_parse(parser* psr)
{
	psr->root = parse_stmts(psr);
	return psr->root;
}
