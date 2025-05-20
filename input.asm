    MOV #42, R1
    MOV R1, R2
    MOV (R2), R3
    MOV (R2)+, R4
    MOV -(R3), R5
    MOV 10(R4), R6
    MOV @#1000, R7
    MOV @DATA, PC
    ADD R1, R2
    SUB R3, R4
    CMP R5, #100
    CLR R0
    COM R1
    INC R2
    DEC R3
    NEG R4
    JMP EXIT
    RTS R5
    HALT
EXIT:   .WORD 0177777
DATA:   .BYTE 1, 2, 3, 4
STR:    .ASCII "HELLO"
        .EVEN
END:    .END