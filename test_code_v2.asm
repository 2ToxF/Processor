push 0
pop  AX

NEXT:
push AX
push AX
mul
out

push AX
push 1
add
pop  AX

push AX
push 10
jb NEXT           ; if AX < 10, jump to NEXT-label

hlt
