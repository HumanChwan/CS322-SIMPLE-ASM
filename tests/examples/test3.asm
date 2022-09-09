ldc 0x100 ; initial value
a2sp ;stack move
ldc times ; count
adj -1
stl 0
LOOP: ; loop start
    ldl 0 ; load left count
    brz LOOP_OUT ; check
    ldc 1 ; decrement
    sub ; left count - 1
    stl 0 ; store count left
    ldc result ; load memory addr of result
    ldnl 0 ; load value of data
    adc increment ; add increment
    ldc result ; load addr, and B = A
    stnl 0 ; store again
    br LOOP
LOOP_OUT:
times: SET 2
increment: SET 3

HAHA:
HALT
result: data 0