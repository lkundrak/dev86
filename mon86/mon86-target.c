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

err_t recv_char (char_t * c)
	{
	ssize_t n = read (0, c, 1);
	return (n == 1) ? E_OK : E_END;
	}

err_t send_char (char_t c)
	{
	ssize_t n = write (1, &c, 1);
	return (n == 1) ? E_OK : E_END;
	}

err_t send_string (char_t * str, word_t count)
	{
	ssize_t n = write (1, str, count);
	return (n == count) ? E_OK : E_END;
	}


static byte_t _mem [0x100000];


static addr_t addr_seg_off (word_t segment, word_t offset)
	{
	return ((addr_t) segment << 4) + (addr_t) offset;
	}


static void mem_read (context_t * context)
	{
	word_t offset;

	offset = context->offset;
	context->value = (word_t) _mem [addr_seg_off (context->segment, offset)];
	context->offset = ++offset;
	}

static void mem_write (context_t * context)
	{
	word_t offset;

	offset = context->offset;
	_mem [addr_seg_off (context->segment, offset)] = (byte_t) context->value ;
	context->offset = ++offset;
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
		index = hex_to_digit (context->token [1]);
		if (index >= REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		context->value = *((reg_t *) regs + index);
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
		index = hex_to_digit (context->token [1]);
		if (index >= REG_MAX)
			{
			err = E_INDEX;
			break;
			}

		*((reg_t *) regs + index) = context->value;
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

	context.offset = 0;
	context.segment = 0xF000;
	context.value = 0;
	context.count = 1;
	context.done = 0;

	reg_setup (&regs);

	// The magic number is NOP-IRET
	// to return immediately upon INT FFh

	globals.glob_magic = 0xCF90;
	globals.slave_ret = 1;
	globals.slave_run = 0;

	int_setup (&globals);

	// Startup banner

	send_char ('M');
	send_char ('O');
	send_char ('N');
	send_char ('8');
	send_char ('6');
	send_char ('.');
	send_char ('0');
	send_char (13);  // carriage return
	send_char (10);  // line feed

	while (1)
		{
		err = recv_context (&context);
		if (err == E_OK && context.length && ! context.done)
			{
			switch (context.token [0])
				{
				// Read from memory

				case C_MEM_READ:
					if (context.length != 1)
						{
						err = E_LENGTH;
						break;
						}

					mem_read (&context);
					send_word (context.value);

					context.done = 1;
					break;

				// Write to memory

				case C_MEM_WRITE:
					if (context.length != 1)
						{
						err = E_LENGTH;
						break;
						}

					mem_write (&context);
					context.done = 1;
					break;

				// Read register

				case C_REG_READ:
					if (context.length != 2)
						{
						err = E_LENGTH;
						break;
						}

					err = reg_read (&context, &regs);
					if (err) break;

					send_word (context.value);

					context.done = 1;
					break;

				// Write register

				case C_REG_WRITE:
					if (context.length != 2)
						{
						err = E_LENGTH;
						break;
						}

					err = reg_write (&context, &regs);
					context.done = 1;
					break;

				// Call procedure

				case C_PROC:
					if (context.length != 1)
						{
						err = E_LENGTH;
						break;
						}

					proc_call (&context, &regs);
					context.done = 1;
					break;

				// Execute task

				case C_TASK:
					if (context.length != 1)
						{
						err = E_LENGTH;
						break;
						}

					err = task_sub (&globals, &regs);
					context.done = 1;
					break;

				}
			}

		if (err == E_OK && ! context.done) err = E_VALUE;

		send_status (err);

		if (err == E_END) break;
		}
	}
