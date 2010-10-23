
.text
entry start
start:
mov ax,#4
mov bx,#1
mov cx,#hello
mov dx,#endhello-hello
int $80

mov bx,#0
mov ax,#1
int $80

.data
hello:
 .ascii "Hello world!\n"
endhello:
