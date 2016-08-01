      addi $sp, $0, 256
      addi $t5, $0, 2
main:  addi $a0, $zero, 1     #
       jal  do_fib            # fib(1)
       add  $s0, $v0, $zero   # $s0 = fib(1) = 1

       addi $a0, $zero, 4     #
       jal  do_fib            # fib(4)
       add  $s1, $v0, $zero   # $s1 = fib(4) = 5

       addi $a0, $zero, 8    #
       jal  do_fib            # fib(8)
       add  $s2, $v0, $zero   # $s2 = fib(8) = 34
        j exit

do_fib: addi $sp, $sp, -20  # move stack pointer
        sw   $s0, 0($sp)
        sw   $s1, 4($sp)
        sw   $s2, 8($sp)
        sw   $s3, 12($sp)
        sw   $ra, 16($sp)

        add  $s0, $a0, $zero  # $s0 = n
        slt $t0, $s0, $t5      # check base
        bne  $t0, $zero, base # if n < 2 do base


        addi $a0, $s0, -1     # $a0 = n - 1
        jal  do_fib           # fib(n-1)
        add  $s2, $v0, $zero  # $s2 = fib(n-1)
        addi $a0, $s0, -2     # $a0 = n - 2
        jal  do_fib           # fib(n-2)
        add  $s3, $v0, $zero  # $s3 = fib(n-2)
        add  $v0, $s2, $s3    # $v0 = fib(n-1) + fib(n-2)
        j    return           # all done

base:   addi $v0, $zero, 1    # base

return:  lw   $s0, 0($sp)      # pop stack
        lw   $s1, 4($sp)
        lw   $s2, 8($sp)
        lw   $s3, 12($sp)
        lw   $ra, 16($sp)
        addi $sp, $sp, 20     # move stack pointer
        jr   $ra              # return
exit:   addi   $v0, $0 10
