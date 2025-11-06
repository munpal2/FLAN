# IR opcode Á¤º¸

| name | opcode | opr1 | opr2 | ret |
|:---:|:---:|:---:|:---:|:---:|
|IR_ADD| 0 | mvcode | mvcode | mvcode |
|IR_SUB| 1 | mvcode | mvcode | mvcode |
|IR_MUL| 2 | mvcode | mvcode | mvcode |
|IR_DIV| 3 | mvcode | mvcode | mvcode |
|IR_MOD| 4 | mvcode | mvcode | mvcode |
|IR_NEG| 5 | mvcode |  -  | mvcode |
|IR_INC| 6 | mvcode |  -  |  -  |
|IR_DEC| 7 | mvcode |  -  |  -  |
|IR_LOAD| 8 | size | addr | mvcode |
|IR_STORE| 9 | size | addr | mvcode |
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
|IR_ALLOC| 24 | size |  -  | addr |
|IR_FREE| 25 | mvcode |  -  |  -  |
|IR_SYSCALL| 26 | mvcode | arg | mvcode |
|IR_LOADCONST| 27 | size | value | mvcode |
