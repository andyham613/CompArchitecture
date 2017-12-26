.text
mov X1, 0x1000
lsl X1, X1, 16
mov X10, 10
stur X10, [X1, 0x0]
mov X12, 2
stur X12, [X1, 0x10]
ldur X13, [X1, 0x0]
ldur X14, [X1, 0x10]
HLT 0

