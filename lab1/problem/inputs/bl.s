.text
bl foo
add X2, X0, 10
b bar

foo:
add X8, X9, 11
br X30

bar:
add X10, X2, X8
HLT 0

