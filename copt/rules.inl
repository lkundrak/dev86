
mov	ax,%1
mov	bx,%2
call	isr
=
mov	ax,%1
mov	cx,%2
sar	ax,cl
