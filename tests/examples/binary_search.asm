ldc 0x1000                  ; MOVE SP far away
a2sp
call main                   ; call main function
HALT

main:
    adj -1                  ; adjusting SP
    stl 0

    adj -3                  ; adjusting SP for start, end and mid
    ldc 0
    stl 0                   ; storing start
    ldc n
    stl 1                   ; storing end

    LOOP:
        ; check for end case
        ldl 1               ; end
        ldl 0               ; start
        sub                 ; A = end - start
        brlz EXIT           ; if (end <= start) then EXIT
        brz EXIT

        ; calculating mid
        ldl 0
        ldl 1
        add                 ; A := start + end
        ldc 1               ; B := start + end, A := 1
        shr                 ; A := (start + end) >> 1
        stl 2               ; store mid

        ldl 2               ; load mid
        ldc array           ; A = array + 0 and B = mid
        add                 ; A = array + mid
        ldnl 0              ; A = array[mid]

        ldc value           ; A = value and B = array[mid]
        sub                 ; A = array[mid] - value
        brz FOUND

        brlz INC_START
        ldl 2
        stl 1               ; end := mid
        br LOOP

        INC_START:
        ldl 2
        ldc 1
        add                 ; A := mid + 1
        stl 0               ; start := mid + 1
        br LOOP

    FOUND:
    ldl 2                   ; stored value of mid
    ldc searched_index      ; get addr of result
    stnl 0                  ; store at result

    EXIT:
    adj 3                   ; readjust
    ldl 0                   ; retrieve return address
    adj 1                   ; readjust
    return                  ; return

value: SET 91               ; Value to be searched
n: SET 11                   ; length of array
array: data 50              ; array
data 67
data 78
data 87
data 91
data 100
data 203
data 402
data 512
data 516
data 983
searched_index: data -1     ; INDEX
