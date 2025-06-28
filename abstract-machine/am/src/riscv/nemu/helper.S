.globl __am_asm_helper
__am_asm_helper:
# in riscv parameter should in a0~a7,more in stack
    addi sp, sp, -8 //save t0
    sd t0 , 0(sp) 
    mv t0 , a0
    mv a0 , a1
    jalr ra, 0(t0)
    ld t0, 0(sp)
    addi sp, sp, 8
    ret

# in x86 needs save the register parameter in stack 
# then call function