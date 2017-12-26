mov     x28, 0x1000
lsl     x28, x28, 16
abc:
add     x28, x28, #0x10
stur     wzr, [x28,#32]
mov     x0, #0xd2                       // #210
stur     x0, [x28,#20]
stur     wzr, [x28,#12]
b       abc+0x34
ldur     x1, [x28,#32]
ldur     x0, [x28,#12]
add     x0, x1, x0
stur     x0, [x28,#32]
ldur     x0, [x28,#12]
add     x0, x0, #0x1
stur     x0, [x28,#12]
ldur     x1, [x28,#12]
ldur     x0, [x28,#20]
cmp     x1, x0
ble     abc+0x18
ldur    x4, [x28, #32]
sub     x28, x28, #0x10
hlt     0
