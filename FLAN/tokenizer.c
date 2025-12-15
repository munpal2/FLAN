#include "tokenizer.h"

#define KEYWORDS_COUNT 14
#define TYPE_COUNT 5
#define MCMD_COUNT 6

const char* token_strty[] = { "TK_END",         "TK_INT",        "TK_FLOAT",       "TK_STR",          "TK_ID",
                              "TK_INVALID",     "TK_CONST",      "TK_PTR",         "TK_FUNC",        "TK_SEMICOLON",
                              "TK_UINT",        "TK_DECL",       "TK_FOR",         "TK_WHILE",        "TK_IF",
                              "TK_ELSE",        "TK_TRUE",       "TK_FALSE",       "TK_PLUS",         "TK_MINUS",
                              "TK_MUL",         "TK_DIV",        "TK_MOD",         "TK_NOT",          "TK_ASSIGN",
                              "TK_EQ",          "TK_NEQ",        "TK_AND",         "TK_OR",           "TK_BXOR",
                              "TK_BAND",        "TK_BOR",        "TK_BNOT",        "TK_LSHIFT",       "TK_RSHIFT",
                              "TK_INC",         "TK_DEC",        "TK_PLUSEQ",      "TK_MINUSEQ",      "TK_MULEQ",
                              "TK_DIVEQ",       "TK_MODEQ",      "TK_LTE",         "TK_GTE",          "TK_LT",
                              "TK_GT",          "TK_DOT",        "TK_ARROW",       "TK_TYPE",         "TK_SHLEQ",
                              "TK_SHREQ",       "TK_OREQ",       "TK_ANDEQ",       "TK_XOREQ",        "TK_OPEN_PAREN",
                              "TK_CLOSE_PAREN", "TK_OPEN_BRACE", "TK_CLOSE_BRACE", "TK_OPEN_BRACKET", "TK_CLOSE_BRACKET",
                              "TK_COMMA",       "TK_AS",         "TK_ARR",         "TK_OF",           "TK_COLON",
                              "TK_RETURN",      "TK_CHAR",       "TK_TYFLOAT",     "TK_TYBOOL",       "TK_TYCHAR",
                              "TK_TYUINT"};

const char* keywords[KEYWORDS_COUNT] = { "decl",  "for", "while", "if",    "else",
                                         "const", "ptr", "true",  "false", "as", 
                                         "arr",   "of",  "func",  "return"};
token_type keyword_types[KEYWORDS_COUNT] = { TK_DECL,  TK_FOR,  TK_WHILE, TK_IF,    TK_ELSE,
                                             TK_CONST, TK_PTR,  TK_TRUE,  TK_FALSE, TK_AS, 
                                             TK_ARR,   TK_OF,   TK_FUNC,  TK_RETURN };

const char* types[TYPE_COUNT] = { "int", "float", "bool", "uint", "char" };
token_type type_types[TYPE_COUNT] = { TK_TYINT, TK_TYFLOAT, TK_TYBOOL, TK_TYUINT, TK_TYBOOL };

const char* specs = ";{}[](),.~:";
token_type spec_types[] = { TK_SEMICOLON,  TK_OPEN_BRACE,  TK_CLOSE_BRACE, TK_OPEN_BRACKET, TK_CLOSE_BRACKET,
                            TK_OPEN_PAREN, TK_CLOSE_PAREN, TK_COMMA,       TK_DOT,          TK_BNOT,
                            TK_COLON};

const char* macro_cmds[MCMD_COUNT] = {"define", "undef", "include", "ifdef", "ifndef", "endif"};
typedef enum macro_type
{
    DEFINE, UNDEF, INCLUDE, IFDEF, IFNDEF, ENDIF
} macro_type;

void token_create(token* dest,
                  token_type type,
                  unsigned int col,
                  char* attr,
                  char* filename)
{
    dest->type = type;
    dest->col = col;
    dest->attr = pool_intern(&global_strpool, attr);
    dest->filename = pool_intern(&global_strpool, filename);
}

typedef struct fileref_context 
{
    file_poller fpl;
    const char* file_name;
    unsigned int col;
} fileref_context;

static void frctx_create(fileref_context* dest, const char* filename)
{
    fpl_open(&(dest->fpl), filename, "r");
    dest->file_name = _strdup(filename);
    dest->col = 1;
}

static void frctx_destroy(fileref_context* dest)
{
    free(dest->file_name);
    fpl_close(&(dest->fpl));
}

static inline bool frctx_end(fileref_context* dest)
{
    return (fpl_lookahead(&(dest->fpl), 0) == EOF);
}

typedef struct strref_context
{
    const char* str;
    size_t offset;
} strref_context; //전부 다 빌려오는거임 할당 해제 안해도 됨

static void srctx_create(strref_context* dest, const char* str)
{
    dest->str = str;
    dest->offset = 0;
}

bool tknz_init(tokenizer* tknz, const char* filename)
{
    varr_create(&(tknz->result), token, 100);
    varr_create(&(tknz->macro_condit), bool, 16);
    varr_create(&(tknz->fileref_stack), fileref_context, 8);
    frctx_create(varr_push(&(tknz->fileref_stack)), filename);
    varr_create(&(tknz->strref_stack), strref_context, 16);

    htb_create(&(tknz->macro_map), const char*);
    htb_create(&(tknz->token_map), token_type);
    htb_create(&(tknz->mcmd_map), macro_type);

    token_type* node;
    for (size_t i = 0; i < KEYWORDS_COUNT; i++)
    {
        node = htb_insert(&(tknz->token_map), keywords[i]);
        *node = keyword_types[i];
    }

    for (size_t i = 0; i < TYPE_COUNT; i++)
    {
        node = htb_insert(&(tknz->token_map), types[i]);
        *node = type_types[i];
    }

    for (size_t i = 0; i < MCMD_COUNT; i++)
    {
        *(macro_type*)(htb_insert(&(tknz->mcmd_map), macro_cmds[i])) = i;
    }
    return true;
}

static void free_strp(const char** strp) { free(*strp); }
void tknz_destroy(tokenizer* tknz)
{
    file_poller* fpl;
    while ((fpl = varr_back(&(tknz->fileref_stack))) != NULL)
    {
		frctx_destroy(fpl);
		varr_pop(&(tknz->fileref_stack));
    }

    varr_destroy(&(tknz->result));
    varr_destroy(&(tknz->fileref_stack));
    varr_destroy(&(tknz->strref_stack));
    varr_destroy(&(tknz->macro_condit));

    htb_destroy(&(tknz->token_map));
    htb_destroy(&(tknz->mcmd_map));
    htb_foreach(&(tknz->macro_map), free_strp);
    htb_destroy(&(tknz->macro_map));
}

static strref_context* srctx_top(tokenizer* tknz) //스택이 비었으면 NULL
{
    while (tknz->strref_stack.size != 0)
    {
        strref_context* srctx = varr_back(&(tknz->strref_stack));
        if (srctx->str[srctx->offset] != '\0')
            return srctx;
        varr_pop(&(tknz->strref_stack));
    }
    return NULL;
}

static fileref_context* frctx_top(tokenizer* tknz)
{
    while (tknz->fileref_stack.size > 1)
    {
        fileref_context* frctx = varr_back(&(tknz->fileref_stack));
        if (!frctx_end(frctx))
            return frctx;
        frctx_destroy(frctx);
        varr_pop(&(tknz->fileref_stack));
    }
    return varr_back(&(tknz->fileref_stack)); //이러면 망하지 않을까?
}

static void push_err(tokenizer* tknz, const char* str)
{
    fileref_context* top = frctx_top(tknz);
	printf(color_err "%s(Line %u): %s", top->file_name, top->col, str); 
	abort();
}

static char tknz_get(tokenizer* tknz)
{
    strref_context* srctx = srctx_top(tknz);
    if (srctx != NULL)
        return srctx->str[srctx->offset];
    fileref_context* frctx = frctx_top(tknz);
    return fpl_lookahead(&(frctx->fpl), 0);
}

static void tknz_pop(tokenizer* tknz)
{
    strref_context* srctx = srctx_top(tknz);
    if (srctx != NULL)
        return srctx->offset++;
    fileref_context* frctx = frctx_top(tknz);
    fpl_dispose(&(frctx->fpl), 1);
}

static inline char tknz_poll(tokenizer* tknz)
{
    strref_context* srctx = srctx_top(tknz);
    if (srctx != NULL)
        return srctx->str[srctx->offset++];
    fileref_context* frctx = frctx_top(tknz);
    return fpl_poll(&(frctx->fpl));
}

static inline bool isdigit(char ch) 
{
    return ('0' <= ch && ch <= '9');
}

static inline bool isalpha(char ch)
{
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

//ch가 str에서 몇 번째에 나오는지를 구함, ch가 없으면 -1
static inline int search_str(char ch, const char* str)
{
    for (int i = 0; str[i]; i++)
    {
        if (str[i] == ch)
            return i;
    }
    return -1;
}

static inline void push_token(tokenizer* tknz, unsigned int type, const char* attr)
{
    fileref_context* top = frctx_top(tknz);
    token_create(varr_push(&(tknz->result)), type, top->col, attr, top->file_name);
}

static inline void increase_col(tokenizer* tknz)
{
    fileref_context* top = frctx_top(tknz);
    top->col++;
}

static unsigned int assume_numtkty(const char* str)
{
    bool dot_included = false;
    size_t dot_pos = -1;
    for (size_t i = 1; str[i] != '\0'; i++)
    {
        switch (str[i])
        {
        case '.':
            if (dot_included || str[i + 1] == '\0')
                return TK_INVALID;
            dot_included = true;
        }
    }

    if (dot_included)
        return TK_FLOAT;
    return TK_INT;
}

typedef struct op_info {
    char mark; //ex) +
    token_type basic; //ex) + 
    token_type basic_eq; //ex) +=
    token_type complex; //ex) ++
    token_type complex_eq; //ex) ++=
} op_info;

const char* marks = "+*/%|&^!=<>";
op_info ops[] = { {'+', TK_PLUS,   TK_PLUSEQ, TK_INC,     TK_INVALID},
                  {'*', TK_MUL,    TK_MULEQ,  TK_INVALID, TK_INVALID},
                  {'/', TK_DIV,    TK_DIVEQ,  TK_INVALID, TK_INVALID},
                  {'%', TK_MOD,    TK_MODEQ,  TK_INVALID, TK_INVALID},
                  {'|', TK_BOR,    TK_OREQ,   TK_OR,      TK_INVALID},
                  {'&', TK_BAND,   TK_ANDEQ,  TK_AND,     TK_INVALID },
                  {'^', TK_BXOR,   TK_XOREQ,  TK_INVALID, TK_INVALID},
                  {'!', TK_NOT,    TK_NEQ,    TK_INVALID, TK_INVALID},
                  {'=', TK_ASSIGN, TK_EQ,     TK_EQ,      TK_INVALID},
                  {'<', TK_LT,     TK_LTE,    TK_LSHIFT,  TK_SHLEQ},
                  {'>', TK_GT,     TK_GTE,    TK_RSHIFT,  TK_SHREQ}};

static char handle_escape(char next)
{
    switch (next)
    {
    case 'n': //\n
        return '\n';
    default:
        return next;
    }
}

static void handle_operator(tokenizer* tknz, op_info* op) 
{
    tknz_pop(tknz);
    char ch = tknz_get(tknz);
    if (ch == '=')
    {
        push_token(tknz, op->basic_eq, NULL);
        tknz_pop(tknz);
    }
    else if (ch == op->mark)
    {
        tknz_pop(tknz);
        ch = tknz_get(tknz);
        if (ch == '=')
        {
            push_token(tknz, op->complex_eq, NULL);
            tknz_pop(tknz);
        }
        else
            push_token(tknz, op->complex, NULL);
    }
    else
        push_token(tknz, op->basic, NULL);
}

static void handle_macro(tokenizer* tknz) 
{
	tknz_pop(tknz); //'#'제거
	str_builder builder;
	str_builder_create(&builder);
	char ch;
    while ((ch = tknz_get(tknz)) != ' ' && ch != EOF && ch != '\n')
    {
        str_builder_add(&builder, ch);
        tknz_pop(tknz);
	} // 매크로 입력
    if (ch == ' ') tknz_pop(tknz); //공백은 무시

	//매크로 처리
	const char* command = _strdup(str_builder_pop(&builder));
	macro_type* mtype = htb_find(&(tknz->mcmd_map), command);
    if (mtype == NULL)
		return push_err(tknz, "알 수 없는 매크로 커맨드입니다.");
    
    while ((ch = tknz_get(tknz)) != ' ' && ch != '\n' && ch != EOF)
    {
        str_builder_add(&builder, ch);
        tknz_pop(tknz);
    } 
    if (ch == ' ') tknz_pop(tknz); //공백은 무시
    const char* name = _strdup(str_builder_pop(&builder)); //매크로 이름

    while ((ch = tknz_get(tknz)) != '\n' && ch != EOF)
    {
        str_builder_add(&builder, ch);
        tknz_pop(tknz);
    }
    const char* value = str_builder_pop(&builder); //매크로 값

    switch (*mtype)
    {
	    case DEFINE:
        {
            char** dest = htb_find(&(tknz->macro_map), name);
            if (dest == NULL)
                dest = htb_insert(&(tknz->macro_map), name);
            else
                free(*dest);
            *dest = _strdup(value);
            break;
        }
        case INCLUDE:
        { //그냥 스택 크기 작으니까 순회할생각
            for (size_t i = 0; i < tknz->fileref_stack.size; i++)
            {
                fileref_context* frctx = varr_get(&(tknz->fileref_stack), i);
                if (strcmp(frctx->file_name, name) == 0) //순환참조 컷
                {
                    printf(color_err "%s에 대한 순환 참조가 있습니다.", name);
                    abort();
                }
            }
            frctx_create(varr_push(&(tknz->fileref_stack)), name);
            break;
        }
        case UNDEF:
        {
            htb_delete(&(tknz->macro_map), name);
            break;
        }
        case IFDEF:
        case IFNDEF:
        {
            bool this_cond = (htb_find(&(tknz->macro_map), name) != NULL);
            if (*mtype == IFNDEF) //ifndef는 부정하기
                this_cond = !this_cond;

            bool prev_cond = tknz->macro_condit.size == 0 ? true : *(bool*)varr_back(&(tknz->macro_condit));
            bool final_cond = this_cond && prev_cond;
            *(bool*)varr_push(&(tknz->macro_condit)) = final_cond;
            break;
        }
        case ENDIF:
        {
            if (tknz->macro_condit.size == 0)
                push_err(tknz, "#endif에 대응되는 #ifdef나 #ifndef가 존재하지 않습니다.");
            varr_pop(&(tknz->macro_condit));
            break;
        }
    }
    str_builder_destroy(&builder);
    free(command);
    free(name);
}

const variable_arr* tokenize(tokenizer* tknz)
{
    str_builder builder;
    int idx;
    char ch;

	str_builder_create(&builder);
    while ((ch = tknz_get(tknz)) != EOF)
    {
        if (tknz->macro_condit.size != 0 && *(bool*)varr_back(&(tknz->macro_condit)) == false) //#ifdef, ifndef등에 대해 비활성화되는 부분을 처리
        {
            while ((ch = tknz_get(tknz)) && ch != '#' && ch != EOF)
            {
                if (ch == '\n')
                    increase_col(tknz);
                tknz_pop(tknz);
            }
        }
 
        if (isdigit(ch))
        {
            do
            {
                str_builder_add(&builder, ch);
                tknz_pop(tknz);
                ch = tknz_get(tknz);
            } while (isdigit(ch) || (ch == '.'));

            token_type assumed_type = assume_numtkty(str_builder_get(&builder)); //토큰이 정상적인지 (1.2333-33 <- ㅇㅈㄹ이 아닌지) 체크
            if (assumed_type == TK_INVALID)
                push_err(tknz, "비정상적인 리터럴이 있습니다.");
            if (assumed_type == TK_INT && (ch == 'U' || ch == 'u')) //INT토큰 뒤에 U가 붙으면 UINT
            {
                str_builder_add(&builder, ch);
                tknz_pop(tknz);
                push_token(tknz, TK_UINT, str_builder_pop(&builder));
            }
            else
                push_token(tknz, assumed_type, str_builder_pop(&builder));
        }

        else if (isalpha(ch) || (ch == '_'))
        {
            do
            {
                str_builder_add(&builder, ch);
                tknz_pop(tknz);
                ch = tknz_get(tknz);
            } while ((isdigit(ch) || isalpha(ch) || (ch == '_')));

            char** macro_val = htb_find(&(tknz->macro_map), str_builder_get(&builder));
            if (macro_val != NULL) //이 토큰이 매크로
            {
                srctx_create(varr_push(&(tknz->strref_stack)), *macro_val); // 이거 읽기 시작
                str_builder_pop(&builder);
            }
            else //매크로 아님
            {
                token* htbfind = htb_find(&(tknz->token_map), str_builder_get(&builder));
                if (htbfind != NULL) //이미 보았던 것
                    push_token(tknz, htbfind->type, str_builder_pop(&builder));
                else
                    push_token(tknz, TK_ID, str_builder_pop(&builder));
            }
        }

        else if ((idx = search_str(ch, specs)) != -1) //specs = ";{}[](),.~"
        {
            tknz_pop(tknz); 
            push_token(tknz, spec_types[idx], NULL);
        }

        else if ((idx = search_str(ch, marks)) != -1) //marks = "+*/%|&^!=<>"
        {
            handle_operator(tknz, ops + idx);
        }

        else switch (ch)
        {
            case ' ': case '\t':
            {
                tknz_pop(tknz);
                break;
            }

            case '\n':
            {
                increase_col(tknz);
                tknz_pop(tknz);
                break;
            }

            case '"':
            {
                tknz_pop(tknz);
                while ((ch = tknz_poll(tknz)) != '"' && ch != EOF)
                {
                    if (ch == '\\')
                        ch = handle_escape(tknz_poll(tknz)); //이스케이프 처리
                    str_builder_add(&builder, ch);
                }
                push_token(tknz, TK_STR, str_builder_pop(&builder));
                break;
            }

            case '\'':
            {
                tknz_pop(tknz);
                ch = tknz_poll(tknz);
                if (ch == '\\')
                    ch = handle_escape(tknz_poll(tknz)); //이스케이프 처리
                str_builder_add(&builder, ch);
                if (tknz_poll(tknz) != '\'')
                    push_err(tknz, "\'(작은따옴표)안에는 하나의 아스키 문자만 허용됩니다.");
                push_token(tknz, TK_CHAR, str_builder_pop(&builder));
                break;
            }

            case '$':
            {
                while ((ch = tknz_poll(tknz)) != '\n' && ch != EOF);
                increase_col(tknz);
                break;
            }

            case '-':
            {
                tknz_pop(tknz);
                ch = tknz_get(tknz);
                unsigned int type = TK_MINUS;
                switch (ch)
                {
                case '-':
                    type = TK_DEC;
                    break;
                case '=':
                    type = TK_MINUSEQ;
                    break;
                case '>':
                    type = TK_ARROW;
                    break;
                }
                if (type != TK_MINUS)
                    tknz_pop(tknz);
                push_token(tknz, type, NULL);
            }

            case '#':
            {
                handle_macro(tknz);
                break;
			}
        }
    }
    push_token(tknz, TK_END, NULL);
    str_builder_destroy(&builder);
    return &(tknz->result);
}
