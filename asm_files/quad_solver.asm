in
in
in

call QUAD_SOLVER
hlt

QUAD_SOLVER:
    pop  CX
    pop  BX
    pop  AX

    push AX
    push 0
    je   LINE_SOLVER

    push BX
    push BX
    mul

    push 4
    push AX
    push CX
    mul
    mul

    sub
    pop DX

    push DX
    push 0
    je DescrZero

    push DX
    push 0
    jb DescrNeg

    push 2           ; Number of roots = 2
    out

    push 0
    push BX
    sub

    push DX
    sqrt
    sub

    push 2
    div

    push AX
    div
    out

    push 0
    push BX
    sub

    push DX
    sqrt
    add

    push 2
    div

    push AX
    div
    out
    ret

    DescrZero:
        push 1       ; Number of roots = 1
        out

        push 0
        push BX
        sub

        push 2
        div

        push AX
        div

        out
        ret

    DescrNeg:
        push 0
        out
        ret






LINE_SOLVER:
    push BX
    push 0
    jne  BXNZERO

    push CX
    push 0
    jne CXNZERO

    push 100000000   ; Number of roots = infinity
    out
    ret

    CXNZERO:
        push 0       ; Number of roots = 0
        out
        ret

    BXNZERO:
        push 1       ; Number of roots = 1
        out

        push 0
        push CX
        sub

        push BX
        div

        out
        ret

