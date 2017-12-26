.text
add X9, X10, 101
subs X10, X9, 100
BGT abc

add X0, X1, 1

abc:
add X7, X8, 101
subs X8, X7, 100
BGE bcd

add X0, X1, 2

bcd:
add X5, X6, 100
subs X6, X5, 2248
BLT cde

add X0, X1, 3

cde:
add X3, X4, 100
subs X4, X3, 101
BLE def

add X0, X1, 4

def:
add X11, X12, 101
subs X12, X11, 101
BEQ efg

add X0, X1, 5

efg: 
add X13, X14, 101
subs X14, X13, 100
BNE fgh

add X0, X1, 6

fgh:

add X0, X1, 7
HLT 0
