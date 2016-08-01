	   	addi $sp, $0, 256
main:	addi	$s0, $0, 12		# Fib(4)
		sw		$s0,0($sp)		# push argument for Fib on stack
		addi	$sp,$sp,-4		#   and decrement stack pointer
		jal		Fib				# jump to subroutine
		addi	$sp,$sp,4		# increment stack pointer
		lw		$s1,0($sp)		#   and pop result from stack
		sw		$s1, 36($0)		#	store the result
		j		Exit			# End of program
Fib:	sw		$ra,0($sp)		# save return address on stack, since recursive,
		addi	$sp,$sp,-4		#   and decrement stack pointer
		sw		$fp,0($sp)		# save previous frame pointer on stack
		addi	$sp,$sp,-4		#   and decrement stack pointer
		addi	$fp,$sp,12		# set frame pointer to point at base of stack frame
		lw		$t0,0($fp)		# copy argument to $t0:  $t0 = n
		addi	$t1, $0, 2
		add		$t3,$0,$0
		slt		$t3,$t1,$t0
		bne     $t3,$0,do_recurse
		addi	$t0, $0, 1		#   else set result to 1 (base cases  n = 1 and  n = 2)
		j		epilogue		# branch to end
do_recurse:	addi	$t0,$t0,-1		# $t0 = n-1
			sw		$t0,0($sp)		# push argument n-1 on stack
			addi	$sp,$sp,-4		#   and decrement stack pointer
			jal		Fib				# call Fibonacci with argument n-1
			lw		$t0,0($fp)		# re-copy argument to $t0:  $t0 = n
			addi	$t0,$t0,-2		# $t0 = n-2
			sw		$t0,0($sp)		# push argument n-2 on stack
			addi	$sp,$sp,-4		#   and decrement stack pointer
			jal		Fib				# call Fibonacci with argument n-2
			addi	$sp,$sp,4		# increment stack pointer
			lw		$t0,0($sp)		#   and pop result of Fib(n-2) from stack into $t0
			addi	$sp,$sp,4		# increment stack pointer
			lw		$t1,0($sp)		#   and pop result of Fib(n-1) from stack into $t1
			add		$t0,$t0,$t1		# $t0 = Fib(n-2) + Fib(n-1); have result
epilogue:	addi	$sp,$sp,4		# increment stack pointer
			lw		$fp,0($sp)		#   and pop saved frame pointer into $fp
			addi	$sp,$sp,4		# increment stack pointer
			lw		$ra,0($sp)		#   and pop return address into $ra
			addi	$sp,$sp,4		# increment stack pointer
			sw		$t0,0($sp)		# push result onto stack
			addi	$sp,$sp,-4		#   and decrement stack pointer
			jr		$ra				# return to caller
Exit:       addi $0 $0 0

