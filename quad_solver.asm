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
    je   LINEEQ

    push BX
    push BX
    mult

    push 4
    push AX
    push CX
    mult
    mult

    sub
    pop DX

    push DX
    push 0
    je DESCRZERO

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

    DESCRZERO:
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





LINEEQ:
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

