# IR opcode Á¤º¸

| name | opcode | arg\[0\](op1) | arg\[1\](op2) | arg\[2\](dest) |
|:---:|:---:|:---:|:---:|:---:|
|IR_ADD| 0 | mvcode | mvcode | mvcode |
|IR_SUB| 1 | mvcode | mvcode | mvcode |
|IR_MUL| 2 | mvcode | mvcode | mvcode |
|IR_DIV| 3 | mvcode | mvcode | mvcode |
|IR_MOD| 4 | mvcode | mvcode | mvcode |
|IR_NEG| 5 | mvcode |  -  | mvcode |
|IR_INC| 6 | mvcode |  -  |  -  |
|IR_DEC| 7 | mvcode |  -  |  -  |
|IR_LOAD| 8 | size | mvcode | mvcode |
|IR_STORE| 9 | size | mvcode | mvcode |
|IR_AND| 10 | mvcode | mvcode | mvcode |
|IR_OR| 11 | mvcode | mvcode | mvcode |
|IR_XOR| 12 | mvcode | mvcode | mvcode |
|IR_LSHF| 13 | mvcode | mvcode | mvcode |
|IR_RSHF| 14 | mvcode | mvcode | mvcode |
|IR_NOT| 15 | mvcode |  -  | mvcode |
|IR_CMP| 16 | mvcode | mvcode | mvcode |
|IR_GT| 17 | mvcode | mvcode | mvcode |
|IR_LT| 18 | mvcode | mvcode | mvcode |
|IR_JMP| 19 | - | line |  -  |
|IR_JZ| 20 | mvcode | line |  -  |
|IR_JNZ| 21 | mvcode | line |  -  |
|IR_CALL| 22 | func | - |  -  |
|IR_RET| 23 |  -  |  -  |  -  |
|IR_ALLOC| 24 | size |  -  | mvcode |
|IR_FREE| 25 | mvcode |  -  |  -  |
|IR_SYSCALL| 26 | mvcode | arg | mvcode |
|IR_LOADCONST| 27 | value | - | mvcode |
|IR_MOVE|28 | mvcode |  -  | mvcode |
|IR_ADDF|29 | mvcode | mvcode | mvcode |
|IR_SUBF|30 | mvcode | mvcode | mvcode |
|IR_MULF|31 | mvcode | mvcode | mvcode |
|IR_DIVF|32 | mvcode | mvcode | mvcode |
|IR_ITOF|33 | mvcode |  -  | mvcode |
|IR_FTOI|34 | mvcode |  -  | mvcode |
|IR_NEGF|35 | mvcode |  -  | mvcode |
|IR_CMPF|36 | mvcode | mvcode | mvcode |
