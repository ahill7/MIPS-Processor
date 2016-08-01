		addi $t0 $0 3
		addi $t1 $0 13
		add $v1 $t0 $t1
		jal test
		addi $t2 $0 5
		addi $t3 $0 7
test:   addi $v0 $t0 0
        addi $t4 $t0 0
end:    addi $0 $0 0
