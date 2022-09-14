; fibonacci (recursive)
ldc 0x1000
a2sp ; moving SP far away
call main ; calling main
HALT ; halting

main:
    adj -1
    stl 0
    ldc ARG
    call fibo
    ldc result
    stnl 0
    ldl 0
    adj 1
    return
fibo:
    ; return address stored
    adj -1
    stl 0

    ; subtracting with 1
    ldc 1
    sub

    ; BASE Case: 1
    brz FIRST

    ; subtracting with 1
    ldc 1
    sub

    ; Base Case: 2
    brz SECOND

    ; NON-BASE Case
    adj -2
    stl 0
    ldl 0

    ; getting fibo(n - 2)
    call fibo
    stl 1 ; A = ...
    ldl 0 ; A = n - 2 

    ; adding 1
    ldc 1
    add ; A = n - 1

    ; getting fibo(n - 1)
    call fibo ; A = f(n - 1)
    ldl 1 ; A = f(n - 2) & B = f(n - 1)
    add ; A = f(n - 2) + f(n - 1)

    ; readjusting
    adj 2

    br EXIT
    SECOND: ldc 1
    br EXIT
    FIRST: ldc 0
    EXIT:
    ldl 0
    adj 1
    return

ARG: SET 10
result: data 0
