addi $t0, $0, 1
addi $t1, $0, 2
slt $t3, $t0, $t1
beq $t3, $0, end
mult $t4, $t1, $t0
end: