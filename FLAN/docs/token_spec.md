# 토큰 타입 정보

## 기본 구조
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_INT       | 1        | 123        | "123"      |
| TK_FLOAT     | 2        | 3.14       | "3.14"     |
| TK_STR       | 3        | "hello"    | "hello"    |
| TK_ID        | 4        | myVar      | "myVar"    |
| TK_UINT      | 10       | 123u       | "123"      |

## 키워드
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_CONST     | 6        | const      | 없음       |
| TK_PTR       | 7        | ptr        | 없음       |
| TK_FUNC      | 8         | func      | 없음       |
| TK_DECL      | 11       | decl       | 없음       |
| TK_FOR       | 12       | for        | 없음       |
| TK_WHILE     | 13       | while      | 없음       |
| TK_IF        | 14       | if         | 없음       |
| TK_ELSE      | 15       | else       | 없음       |
| TK_TRUE      | 16       | true       | 없음       |
| TK_FALSE     | 17       | false      | 없음       |
| TK_TYPE      | 48       | int, float | "int"      |
| TK_AS        | 61       | as         | 없음       |
| TK_ARR       | 62       | arr        | 없음       |
| TK_OF        | 63       | of         | 없음       |

## 연산자 토큰
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_PLUS      | 18       | +          | 없음       |
| TK_MINUS     | 19       | -          | 없음       |
| TK_MUL       | 20       | *          | 없음       |
| TK_DIV       | 21       | /          | 없음       |
| TK_MOD       | 22       | %          | 없음       |
| TK_NOT       | 23       | !          | 없음       |
| TK_ASSIGN    | 24       | =          | 없음       |
| TK_EQ        | 25       | ==         | 없음       |
| TK_NEQ       | 26       | !=         | 없음       |
| TK_AND       | 27       | &&         | 없음       |
| TK_OR        | 28       | \|\|         | 없음       |
| TK_BXOR      | 29       | ^          | 없음       |
| TK_BAND      | 30       | &          | 없음       |
| TK_BOR       | 31       | \|          | 없음       |
| TK_BNOT      | 32       | ~          | 없음       |
| TK_LSHIFT    | 33       | <<         | 없음       |
| TK_RSHIFT    | 34       | >>         | 없음       |

## 증감 및 복합 대입
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_INC       | 35       | ++         | 없음       |
| TK_DEC       | 36       | --         | 없음       |
| TK_PLUSEQ    | 37       | +=         | 없음       |
| TK_MINUSEQ   | 38       | -=         | 없음       |
| TK_MULEQ     | 39       | *=         | 없음       |
| TK_DIVEQ     | 40       | /=         | 없음       |
| TK_MODEQ     | 41       | %=         | 없음       |
| TK_LTE       | 42       | <=         | 없음       |
| TK_GTE       | 43       | >=         | 없음       |
| TK_LT        | 44       | <          | 없음       |
| TK_GT        | 45       | >          | 없음       |
| TK_DOT       | 46       | .          | 없음       |
| TK_ARROW     | 47       | ->         | 없음       |
| TK_SHLEQ     | 49       | <<=        | 없음       |
| TK_SHREQ     | 50       | >>=        | 없음       |
| TK_OREQ      | 51       | \|=         | 없음       |
| TK_ANDEQ     | 52       | &=         | 없음       |
| TK_XOREQ     | 53       | ^=         | 없음       |

## 괄호 및 특수기호
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_SEMICOLON | 9        | ;          | 없음       |
| TK_OPEN_PAREN| 54       | (          | 없음       |
| TK_CLOSE_PAREN| 55      | )          | 없음       |
| TK_OPEN_BRACE| 56       | {          | 없음       |
| TK_CLOSE_BRACE| 57      | }          | 없음       |
| TK_OPEN_BRACKET| 58     | [          | 없음       |
| TK_CLOSE_BRACKET| 59    | ]          | 없음       |
| TK_COMMA     | 60       | ,          | 없음       |
| TK_COLON     | 64       | :          | 없음       |_

## 기타
| 정의명       | 실제 값 | 예시       | [attr]값   |
|:---:|:---:|:---:|:---:|
| TK_END       | 0        | (파일의 끝)        | 없음       |
| TK_INVALID   | 5        | 3.1.4      | 없음       |