; Program to Calculate `1 + 2 + ... + times`

ldc 0x100           ; initial value
a2sp                ;stack move
ldc times           ; count
adj -1
stl 0

LOOP:               ; loop start
    ldl 0           ; load left count
    brz LOOP_OUT    ; check
    ldc result      ; load memory addr of result
    ldnl 0          ; load value of data
    ldl 0
    add
    ldc result      ; load addr, and B = A
    stnl 0          ; store again
    ldl 0
    ldc 1           ; decrement
    sub             ; left count - 1
    stl 0           ; store count left
    br LOOP
LOOP_OUT:

times: SET 100      ; variable
HALT

result: data 0      ; result is stored here
