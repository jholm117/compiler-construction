   .text
   .globl go

go:
   # save callee-saved registers
   pushq %rbx
   pushq %rbp
   pushq %r12
   pushq %r13
   pushq %r14
   pushq %r15

   call _main

   # restore callee-saved registers and return
   popq %r15
   popq %r14
   popq %r13
   popq %r12
   popq %rbp
   popq %rbx
   retq


_main:
   subq $8, %rsp # Allocate spills
   movq $5, %rdi
   movq $5, %rsi
   cmpq %rsi, %rdi
   je _choice1
   jmp _choice2
_choice1:
   call allocate  # runtime system call
   movq %rax, %r8
   movq %r8, 0(%rsp)
   movq 0(%rsp), %r8
   andq $1, %r8
   movq %r8, 0(%rsp)
   movq 0(%rsp), %r8
   cmpq $0, %r8
   je _isArray
   jmp _isnotArray
_isArray:
   movq %rax, %r8
   movq %r8, 0(%rsp)
   movq 0(%rsp), %r8
   movq $7, 8(%r8)
   movq 0(%rsp), %r8
   movq $9, 16(%r8)
   movq 0(%rsp), %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq 0(%rsp), %r8
   movq 0(%r8), %rdi
   movq $_encodeArraySize_ret, -8(%rsp)
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encodeArraySize        # Call to :encodeArraySize
_encodeArraySize_ret:
   movq %rax, %rdi
   call print  # runtime system call
_choice2:
   movq $5, %rdi
   movq $7, %rsi
   movq $_generate3Darray_ret, -8(%rsp)
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _generate3Darray        # Call to :generate3Darray
_generate3Darray_ret:
_isnotArray:
   addq $8, %rsp # Free stack locations
   ret


_encodeArraySize:
   salq $1, %rdi
   addq $1, %rdi
   movq %rdi, %rax
   ret


_generate3Darray:
   call allocate  # runtime system call
   movq $5, %rdi
   movq %rax, %rsi
   call allocate  # runtime system call
   movq $5, %rdi
   movq %rax, %rsi
   call allocate  # runtime system call
   movq %rax, %rdi
   call print  # runtime system call
   ret

