#
# This is a startup for GCC compiling to an ELF executable.
#
#
	.file	"cstartup.s"

# void (*__cleanup)() = 0;

.globl __cleanup
.data
	.align 4
	.type	 __cleanup,@object
	.size	 __cleanup,4
__cleanup:
	.long 0

.globl errno
	.align 4
	.type	 errno,@object
	.size	 errno,4
errno:
	.long 0

# char ** environ;
	.comm	environ,4,4

.text
  .align 16
.globl __cstartup
.type  __cstartup,@function
__cstartup:		# Crt0 startup (Linux style)
  cmpl	$0,(%esp)
  jz	call_exit	# If argc == 0 this is being called by ldd, exit.

  popl %ecx
  movl %esp,%ebx  # Points to the arguments
  movl %esp,%eax
  movl %ecx,%edx
  addl %edx,%edx
  addl %edx,%edx
  addl %edx,%eax
  addl $4,%eax	  # Now points to environ.

  pushl %eax      # envp
  pushl %ebx      # argp
  pushl %ecx      # argc

  # mov	8(%esp),%eax
  mov	%eax,environ

  call	main
  push	%eax		# Main has returned,
call_exit:
  call	exit		# return val and call exit();
bad_exit:
  jmp	bad_exit	# Exit returned !!

# Exit - call __cleanup then _exit

  .align 16
.globl exit
  .type	 exit,@function
exit:
  pushl %ebp
  movl %esp,%ebp
  pushl %ebx
  movl 8(%ebp),%ebx
  movl __cleanup,%eax
  testl %eax,%eax
  je .L8
  pushl %ebx
  call *%eax
  addl $4,%esp
.L8:
  pushl %ebx
  call _exit
  jmp bad_exit

# _exit is an alias for __exit
  .align 16
.globl _exit
  .type	 _exit,@function
_exit:
 jmp __exit
  
