
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "op-class.h"
#include "emu-mem-io.h"
#include "emu-proc.h"
#include "op-exec.h"
#include "emu-serial.h"


// Program main

int main (int argc, char * argv [])
	{
	int exit_code = 0;

	int f = -1;
	int s = -1;

	while (1)
		{
		mem_io_reset ();
		proc_reset ();
		seg_set (SEG_CS, 0);  // TEST: start at 0:0h

		serial_init ();

		// Load binary image file

		if (argc !=  2)
			{
			printf ("usage: %s <file>\n", argv [0]);
			exit_code = 1;
			break;
			}

		f = open (argv [1], O_RDONLY);
		if (f < 0)
			{
			puts ("fatal: cannot open file");
			exit_code = 1;
			break;
			}

		off_t file_size = lseek (f, 0, SEEK_END);
		if (file_size <= 0)
			{
			puts ("fatal: empty file");
			exit_code = 1;
			break;
			}

		printf ("info: file size=%lXh\n", file_size);
		if (file_size > MEM_MAX)
			{
			puts ("fatal: file too big");
			exit_code = 1;
			break;
			}

		lseek (f, 0, SEEK_SET);

		byte_t * buf = mem_get_addr (0);
		off_t read_size = read (f, buf, file_size);
		if (read_size != file_size)
			{
			puts ("fatal: incomplete file read");
			exit_code = 1;
			break;
			}

		puts ("info: image loaded");

		op_code_base = buf;
		int flag_prompt = 0;
		word_t breakpoint = 0xFFFF;
		int flag_exit = 0;

		while (!flag_exit)
			{
			// Decode next instruction

			op_code_seg = seg_get (SEG_CS);
			op_code_off = reg16_get (REG_IP);

			// Breakpoint test

			if (op_code_off == breakpoint)
				{
				putchar ('\n');
				puts ("info: breakpoint hit");
				flag_prompt = 1;
				}

			op_desc_t desc;
			memset (&desc, 0, sizeof desc);
			int err = op_decode (&desc);
			if (err)
				{
				putchar ('\n');
				puts ("error: unknown opcode");
				flag_prompt = 1;
				}

			int flag_exec = 1;

			// User prompt

			if (flag_prompt)
				{
				// Print processor status

				putchar ('\n');
				regs_print ();
				putchar ('\n');

				printf ("%.4X:%.4X ", seg_get (SEG_CS), reg16_get (REG_IP));
				print_column (op_code_str, 3 * OPCODE_MAX + 1);
				op_print (&desc);
				puts ("\n");

				// Get user command
				// Ugly but temporary

				char com [8];
				putchar ('>');
				gets (com);

				switch (com [0])
					{
					// Dump stack

					case 's':
						putchar ('\n');
						stack_print ();
						flag_exec = 0;
						break;

					// Step over

					case 'p':
						breakpoint = op_code_off;
						flag_prompt = 0;
						break;

					// Go (keep breakpoint)

					case 'g':
						flag_prompt = 0;
						break;

					// Quit

					case 'q':
						flag_exec = 0;
						flag_exit = 1;
						break;
					}
				}

			// Execute operation

			if (flag_exec)
				{
				reg16_set (REG_IP, op_code_off);

				err = op_exec (&desc);
				if (err)
					{
					putchar ('\n');
					puts ("fatal: execute operation");
					break;
					}
				}
			}

		break;
		}

	// Cleanup

	if (f >= 0)
		{
		close (f);
		f = -1;
		}

	serial_term ();

	return exit_code;
	}
