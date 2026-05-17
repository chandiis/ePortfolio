# timetemplate.S
# Written 2015 by F Lundevall
# Ported 2024/06 by W Szczerek (from MIPS to RISC-V)
# Copyright abandonded - this file is in the public domain.

#############################################################
# Choose the macro syntax for RARS or DTEK-V board.         #
# The syntax for RARS is probably due to its MIPS heritage. #
#############################################################
###################
# Macros for RARS #
###################
#.macro	PUSH (%reg)
#	addi	sp,sp,-4
#	sw	%reg,0(sp) 
#.end_macro

#.macro	POP (%reg)
#	lw	%reg,0(sp)
#	addi	sp,sp,4
#.end_macro
###################
# Macros for DTEK #
###################
.macro	PUSH reg
	addi sp,sp,-4
	sw \reg,0(sp) 
.endm

.macro	POP reg
	lw	\reg,0(sp)
	addi	sp,sp,4
.endm
#############################################################

	.data
	.align 2
mytime:	.word 	0x5957
timstr:	.asciz 	"text more text lots of text\0"
	.text
	.globl timetemplate, tick, time2string, display_string, delay

# Function for displaying a string with a newline at the end	
display_string:	
	li a7,4
	ecall
	li a0, 10
	li a7,11
	ecall
	jr ra
	
timetemplate:
	la	a0, timstr
	jal     display_string
	
	# wait a little
	li	a0, 1000	# ms, parameter value
	jal	delay
	
	# call tick
	la 	a0, mytime
	jal	tick
	
	# call your function time2string
	la	a0, timstr
	la	t0, mytime
	lw	a1, 0(t0)
	jal	time2string
	
	# go back and do it all again
	j	timetemplate

	
# tick: update time pointed to by $a0
tick:	lw	t0, 0(a0)	# get time
	addi	t0, t0, 1	# increase
	andi	t1, t0, 0xf	# check lowest digit
	sltiu	t2, t1, 0xa	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x6	# adjust lowest digit
	
	andi	t1, t0, 0xf0	# check next digit
	sltiu	t2, t1, 0x60	# if digit < 6, okay
	bnez	t2, tiend
	addi	t0, t0, 0xa0	# adjust digit
	
	li	t3, 0xF
	slli	t3, t3, 0x8
	and	t1, t0, t3	# check minute digit
	addi	t3, x0, 0xA
	slli	t3, t3, 0x8
	slt	t2, t1, t3	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x600	# adjust digit - this one's okay, it's lower than 0x7FF 
	
	li	t3, 0xF
	slli	t3, t3, 0xC
	and	t1, t0, t3	# check last digit
	addi	t3, x0, 0x6
	slli	t3, t3, 0xC
	slt	t2, t1, t3	# if digit < 6, okay
	bnez	t2, tiend
	
	li	t3, 0xA
	slli	t3, t3, 0xC
	add	t0, t0, t3	# adjust last digit
tiend:	sw	t0,0(a0)	# save updated result
	jr	ra		# return

#########################################################
# Place for your functions: time2string, hex2asc, delay.#
#########################################################

time2string:
	PUSH s0
	PUSH s1
	PUSH s2
	PUSH ra
	
	li t0, 0xFFFF    #maskera bort 16 lsb
	and a1, a1, t0
	add s0, a0, zero   #spara adressen
	add s1, a1, zero   #spara tidbitarna
	
	li s2, 12
	
	#1st 
	add a0, s1, zero 
	srl a0, a0, s2
	jal hexasc
	
	sb a0, 0(s0)
	addi s2, s2, -4
	
	#2st
	add a0, s1, zero 
	srl a0, a0, s2
	jal hexasc
	
	sb a0, 1(s0)
	addi s2, s2, -4

	#colon
	li a0, 0x3A
	sb a0, 2(s0)
	
	#3rd
	add a0, s1, zero
	srl a0, a0, s2
	jal hexasc
	
	sb a0, 3(s0)
	addi s2, s2, -4
	
	#4th
	add a0, s1, zero
	jal hexasc
	
	sb a0, 4(s0)
	
	#Null
	sb zero, 5(s0)
	
	POP ra
	POP s2
	POP s1
	POP s0
	
	jr ra

hexasc:
	andi a0, a0, 0xF #bitwise AND to eliminate all bits exept the 4 lsb
	li 	t1, 9	#loads 9 into t1 for counter.
	
	ble 	a0, t1, else	#if it is a number less than or equal to 9, branch
		addi 	a0, a0, 0x37  #adds 0x37 to make a the ASCII-value for that number, A-F
		j 	done
	else:
		addi 	a0, a0, 0x30	#adds 0x30 to make a the ASCII-value for that number, 0-9
	
	done:
		jr	ra

delay:

	li t1, 10000
	dwhile:
		ble a0, zero, dwhileout 
		mv t0, zero
		addi a0, a0, -1
		dfor:
			bge t0, t1, dwhile
			addi t0, t0, 1
			j dfor
			
	dwhileout:
		jr ra