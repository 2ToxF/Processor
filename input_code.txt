push 30
push 70
add                   ; push (pop + pop)
push 60
push 40
sub
div
push 13
push 3
sub
add
out                   ; printf("res = %d", pop)
hlt                   ; остановка процессора
