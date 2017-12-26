.text

    # initialize counter
mov x1, 0
mov x2, 0x1000
lsl x2, x2, 16
    
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
    
    mov x2, 0x1000
    lsl x2, x2, 16
    add x3, x3, x2    

    # console-out syscall
    mov x30, 11
    add x29, x3, x28
    eret

    hlt 0
