// MON86 - Main program

#include "mon86-common.h"


// Host stubbing

#ifdef HOST_STUB
#include <stdio.h>
#endif


// Registers

#define REG_MAX 0x10

typedef word_t reg_t;

struct regs_s
	{
	reg_t ax;  // 0h
	reg_t cx;  // 1h
	reg_t dx;  // 2h
	reg_t bx;  // 3h
	reg_t sp;  // 4h
	reg_t bp;  // 5h
	reg_t si;  // 6h
	reg_t di;  // 7h
	reg_t es;  // 8h
	reg_t cs;  // 9h
	reg_t ss;  // Ah
	reg_t ds;  // Bh
	reg_t ip;  // Ch
	reg_t fl;  // Dh
	reg_t i1;  // Eh
	reg_t i2;  // Fh
	};

typedef struct regs_s regs_t;


// Global data
// Synchronize with mon86-low

struct globals_s
	{
	reg_t master_sp;   // 00h
	reg_t master_ss;   // 02h

	reg_t slave_sp;    // 04h
	reg_t slave_ss;    // 06h

	byte_t slave_run;  // 08h
	byte_t int_num;    // 09h
	};

typedef struct globals_s globals_t;


// Context

struct context_s
	{
	word_t off;
	word_t seg;
	word_t len;
	word_t val;

	globals_t globals;
	regs_t regs;
	};

typedef struct context_s context_t;


// Host stubbing

#ifdef HOST_STUB

err_t read_char (char_t * c)
	{
	ssize_t n = read (0, c, 1);
	return (n == 1) ? E_OK : E_END;
	}

err_t write_char (char_t c)
	{
	ssize_t n = write (1, &c, 1);
	return (n == 1) ? E_OK : E_END;
	}

err_t write_string (char_t * str, word_t len)
	{
	ssize_t n = write (1, str, len);
	return (n == len) ? E_OK : E_END;
	}


static byte_t _mem [0x100000];


static addr_t addr_seg_off (word_t seg, word_t off)
	{
	return ((addr_t) seg << 4) + (addr_t) off;
	}


static void mem_read (context_t * context)
	{
	word_t off;

	off = context->off;
	context->val = (word_t) _mem [addr_seg_off (context->seg, off)];
	context->off = ++off;
	}

static void mem_write (context_t * context)
	{
	word_t off;

	off = context->off;
	_mem [addr_seg_off (context->seg, off)] = (byte_t) context->val ;
	context->off = ++off;
	}


static void sub_call (context_t * context)
	{
	write_string ("CALL ", 5);
	write_word (context->seg);
	write_char (':');
	write_word (context->off);
	write_char (13);
	write_char (10);
	}

static word_t slave_exec (globals_t * globals, regs_t * regs)
	{
	write_string ("TASK", 4);
	write_char (13);
	write_char (10);
	return 3;
	}

static void int_setup (globals_t * globals)
	{
	}

#else // HOST_STUB

extern void mem_write (context_t * context);
extern void mem_read (context_t * context);

extern void sub_call (context_t * context);

extern word_t slave_exec (globals_t * globals, regs_t * regs);

extern void int_setup (globals_t * globals);

#endif // HOST_STUB


// Register operations

static err_t reg_read (context_t * context, char_t suffix)
	{
	err_t err;
	digit_t index;

	reg_t * reg;

	while (1)
		{
		index = hex_to_digit (suffix);
		if (index > REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		reg = (reg_t *) (&context->regs) + index;
		context->val = *reg;

		err = E_OK;
		break;
		}

	return err;
	}


static err_t reg_write (context_t * context, char_t suffix)
	{
	err_t err;
	digit_t index;

	reg_t * reg;

	while (1)
		{
		index = hex_to_digit (suffix);
		if (index > REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		reg = (reg_t *) (&context->regs) + index;
		*reg = context->val;

		err = E_OK;
		break;
		}

	return err;
	}


// Routine execution

static err_t sub_exec (context_t * context, char_t suffix)
	{
	err_t err;
	digit_t index;
	word_t int_num;

	globals_t * globals = &(context->globals);
	regs_t * regs = &(context->regs);

	while (1)
		{
		index = hex_to_digit (suffix);
		switch (index)
			{
			case 0:
			sub_call (context);
			err = E_OK;
			break;

			case 1:
			int_num = slave_exec (globals, regs);
			switch (int_num)
				{
				case 1 : err = E_TRACE; break;
				case 3 : err = E_BREAK; break;
				default : err = E_OK;
				}

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

	context_t context;
	globals_t globals;
	regs_t regs;

	char_t token [TOKEN_LEN_MAX];
	byte_t len;

	context.globals.slave_run = 0;
	int_setup (&context.globals);

	// Startup banner

	write_char ('M');
	write_char ('O');
	write_char ('N');
	write_char ('8');
	write_char ('6');
	write_char ('.');
	write_char ('0');
	write_char (13);  // carriage return
	write_char (10);  // new line

	while (1)
		{
		err = read_token (token, &len);
		if (err == E_OK)
			{
			if (!len) break;

			switch (token [0])
				{
				// Set offset

				case 'O':
					err = hex_to_word (token + 1, len - 1, &context.off);
					break;

				// Set segment

				case 'S':
					err = hex_to_word (token + 1, len - 1, &context.seg);
					break;

				// Set length

				case 'L':
					err = hex_to_word (token + 1, len - 1, &context.len);
					break;

				// Read from memory

				case 'R':
				if (len != 1)
					{
					err = E_LEN;
					break;
					}

				mem_read (&context);

				write_word (context.val);
				write_char (13);  // carriage return
				write_char (10);  // new line
				break;

				// Write to memory

				case 'W':
				if (len != 1)
					{
					err = E_LEN;
					break;
					}

				mem_write (&context);
				break;

				// Read register

				case 'J':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = reg_read (&context, token [1]);
				if (err) break;

				write_word (context.val);
				write_char (13);  // carriage return
				write_char (10);  // new line
				break;

				// Write register

				case 'K':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = reg_write (&context, token [1]);
				break;

				// Execute routine

				case 'X':
				if (len != 2)
					{
					err = E_LEN;
					break;
					}

				err = sub_exec (&context, token [1]);
				break;

				default:
					if (len > 4)
						{
						err = E_LEN;
						break;
						}

					err = hex_to_word (token, len, &context.val);

				}
			}

		write_error (err);

		if (err == E_END) break;
		}
	}
