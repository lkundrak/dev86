
INITSEG = $9000

org 0
mov	ax,#$07c0
mov     ds,ax
mov     ax,#INITSEG
mov     es,ax
mov     cx,#256
sub     si,si
sub     di,di
cld
rep
 movsw
jmpi    go,INITSEG
go:

mov     di,#0x4000-12
mov     ds,ax
mov     ss,ax           ! put stack at INITSEG:0x4000-12.
mov     sp,di

