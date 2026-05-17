//Authors:
//Chandadeep Kaur & Frans Ekberg
.globl strcmp
strcmp:
    lb t0, 0(a0)          # Load byte from str1
    lb t1, 0(a1)          # Load byte from str2

loop:
    bne t0, t1, diff      # If chars differ -> jump to diff
    beqz t0, equal        # If null terminator reached -> equal

    addi a0, a0, 1        # Move to next char in str1
    addi a1, a1, 1        # Move to next char in str2
    lb t0, 0(a0)
    lb t1, 0(a1)
    j loop

equal:
    li a0, 0              # Return 0 (strings equal)
    ret

diff:
    sub a0, t0, t1        # Return difference
    ret