.text
add X10, X9, 10

and X9, X10, 0x01

adds X11, X8, 0xff
and X12, X10, 1
ands X13, X11, 4
sub X9, X10, 5
subs X11, X8, 0xf
sdiv X14, X10, X9
udiv X15, X10, X9
orr X11, X9, X8
lsl X12, X10, 3
lsr X12, X10, 3
mul X8, X10, X12

HLT 0
