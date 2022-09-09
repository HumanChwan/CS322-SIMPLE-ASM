123label:               ; ERROR: illegal label name

add
ldc 0xA
adj                     ; ERROR: expected an operand

a2sp
sp2a

LOOP:                   ; WARNING: unused label
    ldc 4
    adc 1
    add 2               ; ERROR: did not expect an operand
    a2sp
    mult                ; ERROR: unknown mnemonic
    ldnl 5
    br loooooop         ; ERROR: label `looooooop` doesn't exist

br a                    ; WARNING: 0 offset
a: add



