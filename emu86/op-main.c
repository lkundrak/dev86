
#include "op-class.h"
#include "emu-mem-io.h"
#include "emu-proc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


// Program main

int main (int argc, char * argv [])
	{
	int exit_code = 0;

	int f = -1;

	while (1)
		{
		mem_io_reset ();
		proc_reset ();
		seg_set (SEG_CS, 0);  // TEST: start at 0:0h

		// TEST: load test binary

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

		/*
		putchar ('\n');
		print_mem (0, file_size);
		putchar ('\n');
		*/

		op_code_base = buf;
		int flag_prompt = 0;
		word_t breakpoint = 0xFFFF;

		while (1)
			{
			// Decode next instruction

			op_code_seg = seg_get (SEG_CS);
			op_code_off = reg16_get (REG_IP);

			// Breakpoint test

			if (op_code_off == breakpoint)
				{
				puts ("info: breakpoint hit");
				flag_prompt = 1;
				}

			op_desc_t desc;
			memset (&desc, 0, sizeof desc);
			int err = op_decode (&desc);
			if (err)
				{
				puts ("error: unknown opcode");
				flag_prompt = 1;
				}

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
					case 'p':
						breakpoint = op_code_off;
						flag_prompt = 0;
						break;

					case 'g':
						//breakpoint = 0;
						flag_prompt = 0;
						break;

					}

				}

			// Execute operation

			reg16_set (REG_IP, op_code_off);

			err = op_exec (&desc);
			if (err)
				{
				puts ("fatal: execute operation");
				break;
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

	return exit_code;
	}
