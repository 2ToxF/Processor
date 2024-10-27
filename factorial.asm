in
call FACT
out
hlt

FACT:
    pop  BX
    push BX

    push BX
    push 1
    je STOP

    push BX
    push 1
    sub

    call FACT
    mul
    ret

    STOP:
        ret
