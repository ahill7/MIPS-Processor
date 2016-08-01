		addi $t0 $0 3
		addi $t2 $0 13
		jal test
		addi $t1 $0 5
		addi $t3 $0 7
test:
		add $v0 $t2 $0
		jr $ra
end:
