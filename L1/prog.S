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
   movq $_call_label0, -8(%rsp)
   movq $20, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop1        # Call to :loop1
_call_label0:
   movq %rax, %r8
   movq $_call_label1, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label1:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label2, -8(%rsp)
   movq $-20, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop2        # Call to :loop2
_call_label2:
   movq %rax, %r8
   movq $_call_label3, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label3:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label4, -8(%rsp)
   movq $100, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop3        # Call to :loop3
_call_label4:
   movq %rax, %r8
   movq $_call_label5, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label5:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label6, -8(%rsp)
   movq $20, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop4        # Call to :loop4
_call_label6:
   movq %rax, %r8
   movq $_call_label7, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label7:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label8, -8(%rsp)
   movq $248, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop5        # Call to :loop5
_call_label8:
   movq %rax, %r8
   movq $_call_label9, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label9:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label10, -8(%rsp)
   movq $19, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop6        # Call to :loop6
_call_label10:
   movq %rax, %r8
   movq $_call_label11, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label11:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label12, -8(%rsp)
   movq $15, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop7        # Call to :loop7
_call_label12:
   movq %rax, %r8
   movq $_call_label13, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label13:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   movq $_call_label14, -8(%rsp)
   movq $-61254, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _loop8        # Call to :loop8
_call_label14:
   movq %rax, %r8
   movq $_call_label15, -8(%rsp)
   movq %r8, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _encode        # Call to :encode
_call_label15:
   movq %rax, %r8
   movq %r8, %rdi
   call print  # runtime system call
   ret


_loop1:
   movq %rdi, %r10
   movq $0, %r8
_call_label16:
   movq %r10, %r10
   subq $1, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label17
   jmp _call_label16
_call_label17:
   movq %r8, %rax
   ret


_loop2:
   movq %rdi, %r10
   movq $0, %r8
_call_label18:
   movq %r10, %r10
   addq $2, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label19
   jmp _call_label18
_call_label19:
   movq %r8, %rax
   ret


_loop3:
   movq %rdi, %r10
   movq $0, %r8
_call_label20:
   movq %r10, %r10
   sarq $1, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label21
   jmp _call_label20
_call_label21:
   movq %r8, %rax
   ret


_loop4:
   movq %rdi, %r10
   movq $0, %r8
_call_label22:
   movq %r10, %r10
   imulq $0, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label23
   jmp _call_label22
_call_label23:
   movq %r8, %rax
   ret


_loop5:
   movq %rdi, %r10
   movq $0, %r8
_call_label24:
   movq %r10, %r10
   subq $4, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label25
   jmp _call_label24
_call_label25:
   movq %r8, %rax
   ret


_loop6:
   movq %rdi, %r10
   movq $0, %r8
_call_label26:
   movq %r10, %r10
   salq $1, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label27
   jmp _call_label26
_call_label27:
   movq %r8, %rax
   ret


_loop7:
   movq %rdi, %r10
   movq $0, %r8
_call_label28:
   cmpq $1, %r10
   setg %r10b
   movzbq %r10b, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label29
   jmp _call_label28
_call_label29:
   movq %r8, %rax
   ret


_loop8:
   movq %rdi, %r10
   movq $0, %r8
_call_label30:
   movq %r10, %r10
   addq $3, %r10
   movq %r8, %r8
   addq $1, %r8
   cmpq $0, %r10
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label31
   jmp _call_label30
_call_label31:
   movq %r8, %rax
   ret


_encode:
   movq %rdi, %r8
   movq %r8, %r9
   andq $1, %r9
   cmpq $1, %r9
   sete %r9b
   movzbq %r9b, %r9
   cmpq $1, %r9
   je _call_label33
   jmp _call_label32
_call_label32:
   movq %r8, %r8
   addq $1, %r8
_call_label33:
   movq %r8, %rax
   ret

