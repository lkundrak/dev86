// MON86 - Main program

#include "mon86-common.h"


// Host stubbing

#ifdef HOST_STUB
#include <stdio.h>
#endif


// Registers
// Synchronize with mon86-low

#define REG_MAX 0x0E

typedef word_t reg_t;

struct regs_s
	{
	reg_t ax;   // 0h
	reg_t cx;   // 1h
	reg_t dx;   // 2h
	reg_t bx;   // 3h
	reg_t sp;   // 4h
	reg_t bp;   // 5h
	reg_t si;   // 6h
	reg_t di;   // 7h
	reg_t es;   // 8h
	reg_t cs;   // 9h
	reg_t ss;   // Ah
	reg_t ds;   // Bh
	reg_t ip;   // Ch
	reg_t fl;   // Dh
	};

typedef struct regs_s regs_t;


// Global data
// Synchronize with mon86-low

struct globals_s
	{
	word_t glob_magic;  // +0h

	reg_t master_sp;    // +2h
	reg_t master_ss;    // +4h

	reg_t slave_sp;     // +6h
	reg_t slave_ss;     // +8h

	byte_t slave_run;   // +Ah
	byte_t slave_ret;   // +Bh
	};

typedef struct globals_s globals_t;


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


static void proc_call (context_t * context, regs_t * regs)
	{
	}


static word_t task_exec (globals_t * globals, regs_t * regs, word_t start)
	{
	return 255;
	}


static void int_setup (globals_t * globals)
	{
	}


static void reg_setup (regs_t * regs)
	{
	}

#else // HOST_STUB

extern void mem_write (context_t * context);
extern void mem_read  (context_t * context);

extern void proc_call (context_t * context, regs_t * regs);

extern word_t task_exec (globals_t * globals, regs_t * regs, byte_t start);

extern void int_setup (globals_t * globals);
extern void reg_setup (regs_t * regs);

#endif // HOST_STUB


// Register operations

static err_t reg_read (context_t * context, regs_t * regs)
	{
	err_t err;
	digit_t index;

	while (1)
		{
		index = hex_to_digit (context->sub2);
		if (index >= REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		context->val = *((reg_t *) regs + index);
		err = E_OK;
		break;
		}

	return err;
	}


static err_t reg_write (context_t * context, regs_t * regs)
	{
	err_t err;
	digit_t index;

	while (1)
		{
		index = hex_to_digit (context->sub2);
		if (index >= REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		*((reg_t *) regs + index) = context->val;
		err = E_OK;
		break;
		}

	return err;
	}


// Slave operations

static err_t task_sub (globals_t * globals, regs_t * regs)
	{
	err_t err;

	word_t ret;

	// Add return frame if never executed
	// or previously returned (restart)

	if (globals->slave_ret)
		{
		globals->slave_ret = 0;

		ret = 1;
		}
	else
		{
		ret = 0;
		}

	ret = task_exec (globals, regs, ret);

	switch (ret)
		{
		case 1:
			err = E_TRACE;
			break;

		case 3:
			err = E_BREAK;
			break;

		case 255:
			globals->slave_ret = 1;
			err = E_EXIT;
			break;

		default:
			err = E_INDEX;
		}

	return err;
	}


// Program main

void main ()
	{
	err_t err;

	context_t context;
	regs_t regs;
	globals_t globals;

	// Default safe values

	context.off = 0;
	context.seg = 0xF000;
	context.val = 0;
	context.len = 1;

	reg_setup (&regs);

	// The magic number is NOP-IRET
	// to return immediately upon INT FFh

	globals.glob_magic = 0xCF90;
	globals.slave_ret = 1;
	globals.slave_run = 0;

	int_setup (&globals);

	// Startup banner

	write_char ('M');
	write_char ('O');
	write_char ('N');
	write_char ('8');
	write_char ('6');
	write_char ('.');
	write_char ('0');
	write_char (13);  // carriage return
	write_char (10);  // line feed

	while (1)
		{
		err = read_context (&context);
		if (err == E_OK && context.sub1)
			{
			switch (context.sub1)
				{
				// Read from memory

				case C_MEM_READ:
					if (context.sub2)
						{
						err = E_LENGTH;
						break;
						}

					mem_read (&context);
					write_word (context.val);
					write_char (13);  // carriage return
					write_char (10);  // line feed
					break;

				// Write to memory

				case C_MEM_WRITE:
					if (context.sub2)
						{
						err = E_LENGTH;
						break;
						}

					mem_write (&context);
					break;

				// Read register

				case C_REG_READ:
					err = reg_read (&context, &regs);
					if (err) break;

					write_word (context.val);
					write_char (13);  // carriage return
					write_char (10);  // line feed
					break;

				// Write register

				case C_REG_WRITE:
					err = reg_write (&context, &regs);
					break;


				// Call procedure

				case C_PROC:
					if (context.sub2)
						{
						err = E_LENGTH;
						break;
						}

					proc_call (&context, &regs);
					break;

				// Execute task

				case C_TASK:
					if (context.sub2)
						{
						err = E_LENGTH;
						break;
						}

					err = task_sub (&globals, &regs);
					break;

				default:
					err = E_VALUE;

				}
			}

		write_error (err);

		if (err == E_END) break;
		}
	}
