		addi $t1, $0, 4 
		addi $t2, $0, 5
		jal end
		mult $t6, $t1, $t2
#sw   $t4, 4($t5)
label:	
		j stop
end:	mult $t5, $t1, $t2
		mult $t3, $t1, $t2
		mult $t4, $t1, $t2
		j label
stop: