### COMP1521 20T3: ass1: mips_sim ###


An implementation in C of the following 10 MIPS instructions:

Assembler	C	Bit Pattern
add $d, $s, $t	d = s + t	000000ssssstttttddddd00000100000

sub $d, $s, $t	d = s - t	000000ssssstttttddddd00000100010

slt $d, $s, $t	d = s < t	000000ssssstttttddddd00000101010

mul $d, $s, $t	d = s * t	011100ssssstttttddddd00000000010

beq $s, $t, I	if (s == t) PC += I	000100ssssstttttIIIIIIIIIIIIIIII

bne $s, $t, I	if (s != t) PC += I	000101ssssstttttIIIIIIIIIIIIIIII

addi $t, $s, I	t = s + I	001000ssssstttttIIIIIIIIIIIIIIII

ori $t, $s, I	t = s | I	001101ssssstttttIIIIIIIIIIIIIIII

lui $t, I	t = I << 16	00111100000tttttIIIIIIIIIIIIIIII

syscall	syscall	00000000000000000000000000001100


The instruction bit pattern uniquely identifies each instruction:

0: Literal bit zero

1: Literal bit one

I: Immediate (16-bit signed number)

d, s, t: five-bit register number

System Calls

Description	$v0	Pseudo-C

print integer	1	printf("%d", $a0)

exit	10	exit(0)

print character	11	printf("%c", $a0)

Syscall 11 should print the low byte (lowest 8 bits) of $a0.

