ldc 0x1000
a2sp
call main
HALT

main:
    adj -2
    stl 0                   ; Storing return addr
    ldc n
    stl 1
    LOOP_OUTER:
        ldl 1
        brz EXIT_OUTER
        adc -1              ; decrement loop counter (i)
        stl 1
        adj -1              ; INNER loop counter and swap
        ldc 0
        stl 0
        LOOP_INNER:
            ldl 0           ; load inner loop counter (j)
            ldc n
            adc -1          ; A = n - 1
            sub             ; A = j - (n - 1)
            brz EXIT_INNER
            adj -3

            ldc array
            ldl 3           ; j
            add             ; A = array + j
            stl 0           ; store array + j @ loc 0
            ldl 0
            ldnl 0
            stl 2           ; store array[j] @ loc 2

            ldl 0
            adc 1           ; A = array + j + 1
            stl 1           ; store array + j + 1 @ loc 1

            ldl 2           ; A = array[j]
            ldl 1           ; A = array + j + 1 & B = array[j]
            ldnl 0          ; A = array[j + 1] & B = array[j]
            sub             ; A = array[j] - array[j + 1]

            brlz NEXT
            brz NEXT

            ldl 1           ; A = array + j + 1
            ldnl 0          ; A = array[j + 1]
            ldl 0           ; A = array + j & B = array[j + 1]
            stnl 0          ; array[j] = array[j + 1]

            ldl 2           ; A = (prev value)array[j]
            ldl 1           ; A = array + j + 1 & B = (prev value)array[j]
            stnl 0          ; swap complete


            NEXT:
            adj 3
            ldl 0
            adc 1
            stl 0
            br LOOP_INNER
        EXIT_INNER:
        adj 1               ; readjust
        br LOOP_OUTER
    EXIT_OUTER:
    ldl 0
    adj 3
    return

n: SET 9
array: data 100
data 120
data 12
data 54
data 62
data 1
data -3
data 165
data 178
