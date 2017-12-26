.text
mov X1, 0

mov X28, 0
mov X30, 1
eret
cmp X29, X1
bne cpu1

mov X30, 2
eret
cmp X29, X1
bne cpu2

mov X30, 3
eret
cmp X29, X1
bne cpu3

b start

cpu1:
#thread ID 1
    mov X28, 1
    b start
cpu2:
#thread ID 2
    mov X28, 2
    b start
cpu3:
#thread ID 3
    mov X28, 3
    b start

start:
    # all four CPUs come here, with thread ID (0 to 3)
    # console-out syscall
    mov X30, 11
    add X29, X28, 0
    eret

    hlt 0

