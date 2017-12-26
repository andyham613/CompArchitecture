.text
mov X1, 0x1000
lsl X1, X1, 16
mov x0, 0x2174
lsl x0, x0, 32
add x0, x0, 0x126
stur w0, [x1, 0x0]
stur x0, [X1, 0x10]
ldur X13, [X1, 0x0]
ldur X14, [X1, 0x10]
HLT 0

