in
pop  BX

push 1
pop  AX
push 1

call FACT
out
hlt

FACT:
push AX
push BX
jae NEXT

push 1
push AX
add

pop  AX
push AX

mult
call FACT

NEXT:
ret
