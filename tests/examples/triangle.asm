ldc 0x1000          ; move SP far away
a2sp                ; ^
adj -1
ldc result          ; A = addr of result
stl 0               ; stored addr of result
ldc count           ; A = addr of count
ldnl 0              ; A = count
call main           ; A = PC & B = count
adj 1               ; readjust 1
HALT                ; HALT

main:   adj -3              ; make space
        stl 1               ; store PC & A = B = Count
        stl 2               ; store count
        ldc 0               ; A = 0, B = count
        stl 0               ; store 0, A = count

loop:   adj -1              ; make space
        ldl 3               ; A = count
        stl 0               ; stored at 0
        ldl 1               ; A = iter
        call triangle       ; A = PC & B = iter
        ; A -> imp here TODO
        adj 1               ; readjust
        ldl 3               ; load addr of result
        stnl 0              ; store return value from triangle function
        ldl 3               ; load addr of result
        adc 1               ; A = &result + 1
        stl 3               ; store at prev location of result
        ldl 0               ; A = iter
        adc 1               ; A = iter + 1
        stl 0               ; iter = iter + 1
        ldl 0               ; A = iter
        ldl 2               ; A = init_count & B = iter
        sub                 ; A = iter - init_count
        brlz loop           ; if (init_count > iter) then loop
        ldl 1               ; load return address
        adj 3               ; readjust
        return              ; return

triangle:
        adj -3              ; adjust
        stl 1               ; store return address @ loc 1
        stl 2               ; store iter @ loc 2
        ldc 1               ; A = 1 & B = iter
        shl                 ; A = count * 2
        ldl 3               ; A = arg_count & B = iter * 2
        sub                 ; A = iter * 2 - arg_count
        brlz skip           ; if (arg_count > iter * 2) then skip
        ; in else case skip we replaced iter with diff of arg_count with iter
        ldl 3               ; A = arg_count
        ldl 2               ; A = iter & B = arg_count
        sub                 ; A = arg_count - iter
        stl 2               ; store diff @ loc 2
skip:   ldl 2               ; A = sus_val = (iter or diff)
        brz one             ; if (A == 0) then one
        ldl 3               ; A = arg_count & B = sus_val
        adc -1              ; A = arg_count - 1
        stl 0               ; store at 0
        adj -1              ; adjust
        ldl 1               ; A = arg_count - 1
        stl 0               ; store at 0
        ldl 3               ; A = sus_val
        adc -1              ; A = sus_val - 1
        call triangle
        ldl 1               ; A = arg_count - 1 & B = return value
        stl 0               ; A = return value
        stl 1               ; stored return value
        ldl 3               ; A = sus_val
        call triangle
        adj 1
        ldl 0               ; A = prev_return_val & B = return value
        add                 ; A = prev_return_val + return_val
        ldl 1               ; PC
        adj 3               ; readjust
        return              ; return
one:
        ldc 1
        ldl 1
        adj 3
        return
count:  data 10
result: data 0
