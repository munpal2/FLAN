#include "tokenizer.h"

#define KEYWORDS_COUNT 14
#define TYPE_COUNT 5

#define push_err(str) do { \
	printf(color_err "Line %u: " str, tknz->col); \
	abort(); } while(0)

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

void token_create(token* dest, token_type type, unsigned int col, const char* attr)
{
    dest->type = type;
    dest->col = col;
    if (attr != NULL)
        dest->attr = _strdup(attr);
    else
        dest->attr = NULL;
}

static void token_destroy(token* dest)
{
    free(dest->attr);
}

bool tknz_init(tokenizer* tknz, const char* filename)
{
    if (!open_file(&(tknz->fpl), filename, "r") || !varr_create(&(tknz->result), token, 100))
        return false;

    htb_create(&(tknz->token_map), token);

    void* node;
    for (size_t i = 0; i < KEYWORDS_COUNT; i++)
    {
        node = htb_insert(&(tknz->token_map), keywords[i]);
        token_create(node, keyword_types[i], 0, NULL);
    }

    for (size_t i = 0; i < TYPE_COUNT; i++)
    {
        node = htb_insert(&(tknz->token_map), types[i]);
        token_create(node, type_types[i], 0, types[i]);
    }

    tknz->token_cnt = 0;
    tknz->col = 1;
    return true;
}

void tknz_destroy(tokenizer* tknz)
{
    for (size_t i = 0; i < tknz->token_cnt; i++)
        token_destroy(varr_get(&(tknz->result), token, i));
    varr_destroy(&(tknz->result));

    close_file(&(tknz->fpl));
    htb_foreach(&(tknz->token_map), token_destroy);
    htb_destroy(&(tknz->token_map));
}

static inline char tknz_get(tokenizer* tknz)
{
    return lookahead(&(tknz->fpl), 0);
}

static inline void tknz_pop(tokenizer* tknz)
{
    return dispose(&(tknz->fpl), 1);
}

static inline char tknz_poll(tokenizer* tknz)
{
    return poll(&(tknz->fpl));
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
    token_create(varr_get(&(tknz->result), token, tknz->token_cnt++), type, tknz->col, attr);
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

const variable_arr* tokenize(tokenizer* tknz)
{
    str_builder builder;
    int idx;
    char ch;

	str_builder_create(&builder);
    while ((ch = tknz_get(tknz)) != EOF)
    {
        if (isdigit(ch))
        {
            do
            {
                str_builder_add(&builder, ch);
                tknz_pop(tknz);
                ch = tknz_get(tknz);
            } while (isdigit(ch) || (ch == '.'));

            unsigned int assumed_type = assume_numtkty(str_builder_get(&builder)); //토큰이 정상적인지 (1.2333-33 <- ㅇㅈㄹ이 아닌지) 체크
            if (assumed_type == TK_INVALID)
                push_err("비정상적인 리터럴이 있습니다.");
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

            token* htbfind = htb_find(&(tknz->token_map), str_builder_get(&builder));
            if (htbfind != NULL) //이미 보았던 것
                push_token(tknz, htbfind->type, str_builder_pop(&builder));
            else
                push_token(tknz, TK_ID, str_builder_pop(&builder));
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
                tknz->col++;
                tknz_pop(tknz);
                break;
            }

            case '"':
            {
                tknz_pop(tknz);
                while ((ch = tknz_poll(tknz)) != '"' && ch != EOF)
                    str_builder_add(&builder, ch);
                push_token(tknz, TK_STR, str_builder_pop(&builder));
                break;
            }

            case '\'':
            {
                tknz_pop(tknz);
                str_builder_add(&builder, tknz_poll(tknz));
                if ((ch = tknz_poll(tknz)) != '\'')
                    push_err("\'(작은따옴표)안에는 하나의 아스키 문자만 허용됩니다.");
                push_token(tknz, TK_CHAR, str_builder_pop(&builder));
                break;
            }

            case '$':
            {
                while ((ch = tknz_poll(tknz)) != '\n' && ch != EOF);
                tknz->col++;
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
        }
    }
    push_token(tknz, TK_END, NULL);
    str_builder_destroy(&builder);
    return &(tknz->result);
}
