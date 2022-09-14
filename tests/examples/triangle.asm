        ldc 0x1000          ; move SP far away
        a2sp                ; ^
        adj -1
        ldc result          ; A = addr of result
        stl 0               ; stored addr of result
        ldc count           ; A = addr of count
        ldnl 0              ; A = count
        call main           ; A = PC & B = count
        adj 1
        HALT
main:   adj -3              ; make space
        stl 1               ; store PC & A = B = Count
        stl 2               ; store count
        ldc 0               ; A = 0, B = count
        stl 0               ; store 0, A = count
loop:   adj -1              ; make space
        ldl 3               ; A = 0
        stl 0               ; store
        ldl 1               ; 
        call triangle
        adj 1
        ldl 3
        stnl 0
        ldl 3
        adc 1
        stl 3
        ldl 0
        adc 1
        stl 0
        ldl 0
        ldl 2
        sub
        brlz loop
        ldl 1
        adj 3
        return
triangle:adj -3
        stl 1
        stl 2
        ldc 1
        shl
        ldl 3
        sub
        brlz skip
        ldl 3
        ldl 2
        sub
        stl 2
skip:   ldl 2
        brz one
        ldl 3
        adc -1
        stl 0
        adj -1
        ldl 1
        stl 0
        ldl 3
        adc -1
        call triangle
        ldl 1
        stl 0
        stl 1
        ldl 3
        call triangle
        adj 1
        ldl 0
        add
        ldl 1
        adj 3
        return
one:    
        ldc 1
        ldl 1
        adj 3
        return
count:  data 10
result: data 0
