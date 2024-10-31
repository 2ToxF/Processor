in

call FUNC
STOP:
hlt

FUNC:
pop  AX
push AX

push AX
push 6
je STOP

out
ret
