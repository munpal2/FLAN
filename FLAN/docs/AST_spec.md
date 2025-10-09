# AST 노드 타입 정보

## 전체 AST노드 정보
| 정의명        | 실제 값 | 예시             | [attr]값   | [0]           | [1]           | [2]     | [3]     |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| AST_CONJ      | 0        | stmt1 ; stmt2 <br> 1, 2    | 없음       | 앞 구문       | 뒷 구문       |         |         |
| AST_DECL      | 1        | decl int a           | 없음       | 타입          | 식별자        |         |         |
| AST_CONST     | 2        | const int        | 없음       | 붙는 타입     |               |         |         |
| AST_PTROF     | 3        | ptr int          | 없음       | 붙는 타입     |               |         |         |
| AST_ASSIGN    | 4        | x = y            | 없음       | 좌변          | 우변          |         |         |
| AST_ADD       | 5        | x + y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_SUB       | 6        | x - y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_MUL       | 7        | x * y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_DIV       | 8        | x / y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_NEG       | 9        | -x               | 없음       | 대상 식       |               |         |         |
| AST_REF       | 10       | *x               | 없음       | 대상 식       |               |         |         |
| AST_GETADDR   | 11       | &x               | 없음       | 대상 식       |               |         |         |
| AST_ID        | 12       | myVar            | "myVar"    |               |               |         |         |
| AST_STR       | 13       | "hello"          | "hello"    |               |               |         |         |
| AST_INT       | 14       | 123              | "123"      |               |               |         |         |
| AST_FLOAT     | 15       | 3.14             | "3.14"     |               |               |         |         |
| AST_UINT      | 16       | 123u             | "123"      |               |               |         |         |
| AST_TYPE      | 17       | int              | "int"      |               |               |         |         |
| AST_CAST      | 18       | (int)x           | 없음       | 타입          | 대상 식       |         |         |
| AST_ARR       | 19       | arr int[10]     | 없음       | 원소 타입     | 길이 값       |         |         |
| AST_FWINC     | 20       | ++x              | 없음       | 대상 식       |               |         |         |
| AST_FWDEC     | 21       | --x              | 없음       | 대상 식       |               |         |         |
| AST_BWINC     | 22       | x++              | 없음       | 대상 식       |               |         |         |
| AST_BWDEC     | 23       | x--              | 없음       | 대상 식       |               |         |         |
| AST_DOT       | 24       | x.y              | 없음       | 대상 식       | 멤버 ID       |         |         |
| AST_ARROW     | 25       | x->y             | 없음       | 대상 식       | 멤버 ID       |         |         |
| AST_TRUE      | 26       | true             | 없음       |               |               |         |         |
| AST_FALSE     | 27       | false            | 없음       |               |               |         |         |
| AST_NOT       | 28       | !x               | 없음       | 대상 식       |               |         |         |
| AST_BNOT      | 29       | ~x               | 없음       | 대상 식       |               |         |         |
| AST_MOD       | 30       | x % y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_LSHIFT    | 31       | x << y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_RSHIFT    | 32       | x >> y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_LT        | 33       | x < y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_GT        | 34       | x > y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_LTE       | 35       | x <= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_GTE       | 36       | x >= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_EQ        | 37       | x == y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_NEQ       | 38       | x != y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_BAND      | 39       | x & y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_BOR       | 40       | x \| y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_BXOR      | 41       | x ^ y            | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_OR        | 42       | x \|\| y          | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_AND       | 43       | x && y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_ADDI      | 44       | x += y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_SUBI      | 45       | x -= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_MULI      | 46       | x *= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_DIVI      | 47       | x /= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_MODI      | 48       | x %= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_ANDI      | 49       | x &= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_ORI       | 50       | x \|= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_XORI      | 51       | x ^= y           | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_LSHIFTI   | 52       | x <<= y          | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_RSHIFTI   | 53       | x >>= y          | 없음       | 좌측 식       | 우측 식       |         |         |
| AST_WHILE     | 54       | while() \{}       | 없음       | 조건식       | 구문          |         |         |
| AST_IF        | 55       | if () \{}         | 없음       | 조건식       | 구문          | else 일 때의 구문<br>(NULL 일 수 있음) | |
| AST_FDECL     | 56       | func foo(int a, int b): float | 없음 | 함수명 | 매개변수<br>타입 목록<br>(없으면 NULL) | 리턴 타입<br>(없으면 NULL) | 함수 정의 구문
| AST_BLOCK     | 57       | \{}               | 없음 | 블록 내 구문
| AST_PARAM     | 58       | (int a, uint b)   | 없음 | 현재 파라미터 타입 | 현재 파라미터 이름 |
| AST_RETURN    | 59       | return 0;         | 없음 | 리턴값(표현식) 
| AST_IDX       | 60       | x[y]              | 없음 | 배열 역할의 식 | 인덱스 역할의 식
| AST_FUNC      | 61       | func (int, float) | 없음 | 매개변수<br>타입 목록 | 리턴 타입
| AST_CALL      | 62       | foo(1, 2)         | 없음 | 함수 역할의 식 | 인수목록<br>(없으면 NULL)
| AST_FOR       | 63       | for(decl int a, b; a < 0; a + 1) \{} | 없음 | 정의 식 | 반복 지속 조건식 | 증감식 | 구문
| AST_CHAR      | 64       | 'c'               | "c"
| AST_IDINIT    | 65       | list = {1, 2, 3}  | 없음 | 식별자 | 초기화 식
| AST_INITEXPR  | 66       | {1, 2, 3}         | 없음 | 현재 식 | 다음 식<br>(끝이면 NULL) 

## 임시 노드들 (AST optimizing 과정에서 사라짐)
- AST_GTE
- AST_LTE
- AST_NEQ