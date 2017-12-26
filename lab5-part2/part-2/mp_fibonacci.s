.text

    # initialize counter
    # start up CPUs
mov x28, 0
mov x30, 1
eret
cmp x29, x1
bne cpu1

mov x30, 2
eret
cmp x29, x1
bne cpu2

mov x30, 3
eret
cmp x29, x1
bne cpu3

mov x10, 3
b start

cpu1:
#thread ID 1
    mov x10, 5
    b start
cpu2:
#thread ID 2
    mov x10, 10
    b start
cpu3:
#thread ID 3
    mov x10, 15   
    b start
	

start:
    # all four CPUs come here, with thread ID (0 to 3)
    
    mov x1, 2
    mov x2, 3
L1:
    add x3, x1, x2
    add x1, x2, 0
    add x2, x3, 0
    add x10, x10, -1
    cmp x10, 0
    bne L1        

    # console-out syscall
    mov x30, 11
    mov x29, x3
    eret

    hlt 0
