# Rules for optimizing BCC assembler output

# Rules for inlining C library functions

push	word %[#|*]0%1
call	__htons
inc	sp
inc	sp
=
mov	ax,#((%1 & $00FF) << 8) + ((%1 & $FF00) >> 8)

mov	ax,%[#|*]0%1
push	ax
call	__htons
inc	sp
inc	sp
=
mov	ax,#((%1 & $00FF) << 8) + ((%1 & $FF00) >> 8)

push	%0[%1]
call	__htons
inc	sp
inc	sp
=
mov	ax,%0[%1]
xchg	al,ah

push	ax
call	__htons
inc	sp
inc	sp
=
xchg	al,ah

push	%[bx|cx|dx]1
call	__htons
inc	sp
inc	sp
=
mov	ax,%1
xchg	al,ah

call	___get_ds
=
mov	ax,ds

call	___get_es
=
mov	ax,es

call	___get_cs
=
mov	ax,cs

push	word %[#|*]0%1
call	___set_es
inc	sp
inc	sp
=
mov	ax,%0%1
mov	es,ax

mov	ax,%[#|*]0%1
push	ax
call	___set_es
inc	sp
inc	sp
=
mov	ax,%0%1
mov	es,ax

push	%0[%1]
call	___set_es
inc	sp
inc	sp
=
mov	ax,%0[%1]
mov	es,ax

push	%[ax|bx|cx|dx]1
call	___set_es
inc	sp
inc	sp
=
mov	es,%1

push	word %[#|*]0%1
call	___deek_es
inc	sp
inc	sp
=
seg	es
mov	ax,[%1]

mov	ax,%[#|*]0%1
push	ax
call	___deek_es
inc	sp
inc	sp
=
seg	es
mov	ax,[%1]

push	%0[%1]
call	___deek_es
inc	sp
inc	sp
=
mov	bx,%0[%1]
seg	es
mov	ax,[bx]

push	bx
call	___deek_es
inc	sp
inc	sp
=
seg	es
mov	ax,[bx]

push	%[ax|cx|dx]1
call	___deek_es
inc	sp
inc	sp
=
mov	bx,%1
seg	es
mov	ax,[bx]

push	word %[#|*]0%1
call	___peek_es
inc	sp
inc	sp
=
seg	es
mov	al,[%1]
xor	ah,ah

mov	ax,%[#|*]0%1
push	ax
call	___peek_es
inc	sp
inc	sp
=
seg	es
mov	al,[%1]
xor	ah,ah

push	%0[%1]
call	___peek_es
inc	sp
inc	sp
=
mov	bx,%0[%1]
seg	es
mov	al,[bx]
xor	ah,ah

push	bx
call	___peek_es
inc	sp
inc	sp
=
seg	es
mov	al,[bx]
xor	ah,ah

push	%[ax|cx|dx]1
call	___peek_es
inc	sp
inc	sp
=
mov	bx,%1
seg	es
mov	al,[bx]
xor	ah,ah

push	word %[#|*]0%1
call	___poke_es
add	sp,*4
=
pop	ax
seg	es
mov	[%1],al

mov	ax,%[#|*]0%1
push	ax
call	___poke_es
add	sp,*4
=
pop	ax
seg	es
mov	[%1],al

pmov	ax,%[#|*]0%1
push	ax
call	___poke_es
add	sp,*4
=
seg	es
mov	[%1],al

push	%0[%1]
call	___poke_es
add	sp,*4
=
mov	bx,%0[%1]
pop	ax
seg	es
mov	[bx],al

push	bx
call	___poke_es
add	sp,*4
=
pop	ax
seg	es
mov	[bx],al

push	%[ax|cx|dx]1
call	___poke_es
add	sp,*4
=
mov	bx,%1
pop	ax
seg	es
mov	[bx],al

push	word %[#|*]0%1
call	___doke_es
add	sp,*4
=
pop	ax
seg	es
mov	[%1],ax

mov	ax,%[#|*]0%1
push	ax
call	___doke_es
add	sp,*4
=
pop	ax
seg	es
mov	[%1],ax

pmov	ax,%[#|*]0%1
push	ax
call	___doke_es
add	sp,*4
=
seg	es
mov	[%1],ax

push	%0[%1]
call	___doke_es
add	sp,*4
=
mov	bx,%0[%1]
pop	ax
seg	es
mov	[bx],ax

push	bx
call	___doke_es
add	sp,*4
=
pop	ax
seg	es
mov	[bx],ax

push	%[ax|cx|dx]1
call	___doke_es
add	sp,*4
=
mov	bx,%1
pop	ax
seg	es
mov	[bx],ax

push	ax
mov	bx,%1
pop	ax
=
mov	bx,%1

push	%1
pop	ax
=
mov	ax,%1

mov	ax,ax
=
!mov	ax,ax

