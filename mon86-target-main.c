
// Host stubbing

#ifdef HOST_STUB
#include <stdio.h>

#else

// Program entry

#asm

	br _main

#endasm

#endif


// Defines

#define TOKEN_LEN_MAX 4
#define REG_MAX 0x10


// Scalar types

typedef unsigned char digit_t;
typedef unsigned char char_t;
typedef unsigned char byte_t;

typedef unsigned short reg_t;
typedef unsigned long addr_t;


// Error codes

enum err_e
	{
	E_OK,
	E_LEN,
	E_VALUE,
	E_INDEX
	};

typedef enum err_e err_t;


// Context

struct context_s
	{
	reg_t ax;    // 0h
	reg_t bx;    // 1h
	reg_t cx;    // 2h
	reg_t dx;    // 3h
	reg_t si;    // 4h
	reg_t di;    // 5h
	reg_t bp;    // 6h
	reg_t ds;    // 7h
	reg_t es;    // 8h
	reg_t val;   // 9h
	reg_t ip;    // Ah
	reg_t cs;    // Bh
	reg_t fl;    // Ch
	reg_t res1;  // Dh
	reg_t res2;  // Eh
	reg_t res3;  // Fh
	};

typedef struct context_s context_t;


// Serial primitives

#ifdef HOST_STUB

static char_t scan_char ()
	{
	return getchar ();
	}

static void print_char (char_t c)
	{
	putchar (c);
	}

#else // HOST_STUB

#asm

_toggle_0:
	push dx
	push ax
	mov dx, #0xFF74
	in ax, dx
	mov dx, ax
	not ax
	and ax, #1
	and dx, #0xFFFE
	or ax, dx
	mov dx, #0xFF74
	out dx, ax
	pop ax
	pop dx
	ret

_print_char:
	push bp
	mov bp, sp
	push ax
	mov ax, [bp+4]  ; BCC pushes char as word
	mov ah, #0x0A   ; write character
	int 0x10        ; BIOS video service
	pop ax
	pop bp
	ret

_scan_char:
	call _toggle_0  ; quiet the watching thing
	mov ah, #0x10   ; get extended key
	int 0x16        ; BIOS keyboard service
	or ah, ah
	jnz _scan_char  ; retry when no key
	ret

#endasm

#endif // HOST_STUB


// Input helpers

static char_t up_to_down (char_t c)
	{
	if (c >= 'a' && c <= 'z')
		{
		c = c + 'A' - 'a';
		}

	return c;
	}


static err_t scan_token (char_t * token, byte_t * len)
	{
	err_t err;

	char_t *pos;
	byte_t stat;
	char_t c;

	pos = token;
	stat = 0;
	*len = 0;

	err = E_OK;

	while (1)
		{
		c = scan_char ();

		// End of stream

		if (c == 0 || c == 0xFF)
			{
			*len = 0;
			break;
			}

		// TEST : echo
		// print_char (c);

		// Ignore heading spaces
		// Others are separators

		if (c <= ' ')
			{
			if (stat == 0) continue;
			break;
			}

		// Token begins or continues

		stat = 1;

		if (++(*len) > TOKEN_LEN_MAX)
			{
			err = E_LEN;
			}
		else
			{
			*(pos++) = up_to_down (c);
			}
		}

	return err;
	}


static digit_t char_to_digit (char_t c)
	{
	digit_t d;

	d = 0xFF;

	if (c >= '0' && c <= '9') d = c - '0';
	if (c >= 'A' && c <= 'F') d = c - 'A' + 10;

	return d;
	}


// Output helpers

static char_t digit_to_char (digit_t d)
	{
	char_t c;

	if (d > 9)
		{
		c = 'A' + d - 10;
		}
	else
		{
		c = '0' + d;
		}

	return c;
	}


static void print_stat (err_t err)
	{
	print_char ('S');
	print_char (digit_to_char (err & 0xF));
	print_char (13);  // carriage return
	print_char (10);  // new line
	}


static void print_val (reg_t val)
	{
	reg_t mask;
	byte_t start;
	byte_t pos;
	byte_t shift;
	digit_t d;

	mask = 0xF000;
	shift = 12;
	start = 0;

	for (pos = 0; pos < 4; pos++)
		{
		d = (val & mask) >> shift;
		if (d || start || pos == 3)
			{
			start = 1;
			print_char (digit_to_char (d));
			}

		shift -= 4;
		mask >>= 4;
		}

	print_char (13);  // carriage return
	print_char (10);  // new line
	}


// Value and register operations

static err_t val_write (reg_t * regs, char_t * token, byte_t len)
	{
	err_t err;
	char_t *pos;
	digit_t d;

	reg_t val;
	context_t *context;

	context = (context_t *) regs;

	err = E_OK;
	val = 0;

	for (pos = token; pos < token + len; pos++)
		{
		d = char_to_digit (*pos);
		if (d > 0xF)
			{
			err = E_VALUE;
			break;
			}

		val = (val << 4) + d;
		}

	if (err != E_VALUE)
		{
		context->val = val;
		}

	return err;
	}


static err_t reg_read (reg_t * regs, char_t suffix)
	{
	err_t err;
	byte_t index;

	context_t * context;
	reg_t val;

	while (1)
		{
		index = char_to_digit (suffix);
		if (index > REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		val = regs [index];
		context = (context_t *) regs;
		context->val = val;

		err = E_OK;
		break;
		}

	return err;
	}


static err_t reg_write (reg_t * regs, char_t suffix)
	{
	err_t err;
	byte_t index;

	context_t * context;
	reg_t val;

	while (1)
		{
		index = char_to_digit (suffix);
		if (index > REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		context = (context_t *) regs;
		val = context->val;

		regs [index] = val;

		err = E_OK;
		break;
		}

	return err;
	}


// Routine execution

#ifdef HOST_STUB

static byte_t mem [0x100000];

static void sub_read (context_t * context)
	{
	reg_t si, ds, val;
	addr_t addr;

	si = context->si;
	ds = context->ds;
	addr = (ds << 4) + si;

	context->val = mem [addr];
	context->si = ++si;
	}

static void sub_write (context_t * context)
	{
	reg_t di, es;
	addr_t addr;

	di = context->di;
	es = context->es;
	addr = (es << 4) + di;

	mem [addr] = context->val;

	context->di = ++di;
	}

static void sub_call (context_t * context)
	{
	reg_t ip, cs;
	ip = context->ip;
	cs = context->cs;
	printf ("CALL %x:%x\n", cs, ip);
	}

#else // HOST_STUB

#asm

_sub_read:
	push bp
	mov bp,sp
	push ax
	push bx
	push si
	push ds
	mov bx,[bp+4]      ; arg1 = * context
	mov si,[bx+2*0x4]
	mov ds,[bx+2*0x7]
	lodsb
	mov ah,#0
	mov [bx+2*0x9],ax  ; store read value
	mov [bx+2*0x4],si  ; store incremented pointer
	pop ds
	pop si
	pop bx
	pop ax
	pop bp
	ret

_sub_write:
	push bp
	mov bp,sp
	push ax
	push bx
	push di
	push es
	mov bx,[bp+4]      ; arg1 = * context
	mov di,[bx+2*0x5]
	mov es,[bx+2*0x8]
	mov ax,[bx+2*0x9]  ; get value to write
	stosb
	mov [bx+2*0x5],di  ; store incremented pointer
	pop es
	pop di
	pop bx
	pop ax
	pop bp
	ret

_sub_call:
	push bp
	mov bp,sp
	sub sp,#4          ; call local pointer
	push ax
	push bx
	mov bx,[bp+4]      ; arg1 = * context
	mov ax,[bx+2*0xA]    ; IP
	mov [bp-4],ax
	mov ax,[bx+2*0xB]    ; CS
	mov [bp-2],ax
	lea bx,[bp-4]
	call far [bx]
	pop bx
	pop ax
	add sp,#4
	pop bp
	ret

#endasm

#endif // HOST_STUB


extern void slave_exec ();

static sub_slave_exec (context_t * context)
	{
	// TODO: plenty of stuff :-)
	slave_exec ();
	}


static err_t sub_exec (reg_t * regs, char_t suffix)
	{
	err_t err;
	digit_t index;

	context_t * context;
	context = (context_t *) regs;

	err = E_OK;

	while (1)
		{
		index = char_to_digit (suffix);
		switch (index)
			{
			case 0:
			sub_read (context);
			print_val (context->val);
			break;

			case 1:
			sub_write (context);
			break;

			case 2:
			sub_call (context);
			break;

			default:
			err = E_INDEX;
			}

		break;
		}

	return err;
	}


// Program main

void main ()
	{
	err_t err;

	reg_t regs [REG_MAX];
	context_t * context;

	char_t token [TOKEN_LEN_MAX];
	byte_t len;

	context = (context_t *) regs;

	// Startup banner

	print_char ('M');
	print_char ('O');
	print_char ('N');
	print_char ('8');
	print_char ('6');
	print_char ('.');
	print_char ('0');
	print_char (13);  // carriage return
	print_char (10);  // new line

	while (1)
		{
		err = scan_token (token, &len);
		if (err == E_OK)
			{
			if (!len) break;

			switch (token [0])
				{
				case 'R':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = reg_read (regs, token [1]);
				print_val (context->val);
				break;

				case 'W':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = reg_write (regs, token [1]);
				break;

				case 'X':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = sub_exec (regs, token [1]);
				break;

				default:
				if (len > 4)
					{
					err = E_LEN;
					break;
					}

				err = val_write (regs, token, len);
				}
			}

		print_stat (err);
		}
	}
