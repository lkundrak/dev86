
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "op-class.h"

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

#include "op-exec.h"


static int file_load (addr_t start, char * path)
	{
	int err = -1;
	int fd = -1;

	while (1)
		{
		// Load binary image file

		fd = open (path, O_RDONLY);
		if (fd < 0)
			{
			puts ("fatal: cannot open file");
			break;
			}

		off_t size = lseek (fd, 0, SEEK_END);
		if (size <= 0)
			{
			puts ("fatal: empty file");
			break;
			}

		printf ("info: file size=%lXh\n", size);
		if (start + size > MEM_MAX)
			{
			puts ("fatal: file too big");
			break;
			}

		lseek (fd, 0, SEEK_SET);

		byte_t * buf = mem_get_addr (start);
		off_t count = read (fd, buf, size);
		if (count != size)
			{
			puts ("fatal: incomplete file read");
			break;
			}

		puts ("success: file loaded");
		err = 0;
		break;
		}

	// Cleanup

	if (fd >= 0)
		{
		close (fd);
		fd = -1;
		}

	return err;
	}


// Program main

int main (int argc, char * argv [])
	{
	int exit_code = 0;

	while (1)
		{
		mem_io_reset ();
		proc_reset ();

		char * file_path = NULL;
		addr_t file_address = -1;
		int file_loaded = 0;

		addr_t break_code_addr = -1;

		int flag_trace = 0;
		int flag_prompt = 0;

		// Auto check

		int err = check_exec ();
		if (err)
			{
			puts ("fatal: auto check");
			exit_code = -1;
			break;
			}

		// Process command line

		char opt;

		while (1)
			{
			opt = getopt (argc, argv, "w:f:x:c:d:tip");
			if (opt < 0 || opt == '?') break;

			switch (opt)
				{
				case 'w':  // load address
					if (sscanf (optarg, "%lx", &file_address) != 1)
						{
						puts ("error: bad load address");
						}
					else
						{
						printf ("info: load address %.5lXh\n", file_address);
						}

					break;

				case 'f':  // file path
					file_path = optarg;
					printf ("info: load file %s\n", file_path);
					break;

				// Execution address:
				// used to override the default CS:IP at reset

				case 'x':  // execute address
					if (sscanf (optarg, "%hx:%hx", &op_code_seg, &op_code_off) != 2)
						{
						puts ("error: bad execute address");
						}
					else
						{
						printf ("info: execute address %.4hXh:%.4hXh\n", op_code_seg, op_code_off);

						seg_set (SEG_CS, op_code_seg);
						reg16_set (REG_IP, op_code_off);
						}

					break;

				case 'c':  // code breakpoint address
					if (sscanf (optarg, "%lx", &break_code_addr) != 1)
						{
						puts ("error: bad code breakpoint address");
						}
					else
						{
						printf ("info: code breakpoint address %.5lXh\n", break_code_addr);
						}

					break;

				case 'd':  // data breakpoint address
					if (sscanf (optarg, "%lx", &_break_data_addr) != 1)
						{
						puts ("error: bad data breakpoint address");
						}
					else
						{
						printf ("info: data breakpoint address %.5lXh\n", _break_data_addr);
						}

					break;

				case 't':  // trace mode
					flag_trace = 1;
					break;

				case 'i':  // interactive mode
					flag_trace = 1;
					flag_prompt = 1;
					break;

				// Program mode:
				// used when running a stand-alone executable
				// in the tiny memory model where CS=DS=ES=SS

				case 'p':
					seg_set (SEG_DS, seg_get (SEG_CS));
					seg_set (SEG_ES, seg_get (SEG_CS));
					seg_set (SEG_SS, seg_get (SEG_CS));
					break;

				}

			if (file_address != -1 && file_path)
				{
				if (!file_load (file_address, file_path))
					{
					file_loaded = 1;
					}

				file_path = NULL;
				file_address = -1;
				}
			}

		if (opt == '?' || optind != argc || !file_loaded)
			{
			printf ("usage: %s [options]\n\n", argv [0]);
			puts ("  -w <address>         load address");
			puts ("  -f <path>            file path");
			puts ("  -x <segment:offset>  execute address");
			puts ("  -c <address>         code breakpoint address");
			puts ("  -d <address>         data breakpoint address");
			puts ("  -t                   trace mode");
			puts ("  -i                   interactive mode");

			exit_code = 1;
			break;
			}

		// Main loop

		int_init ();
		serial_init ();

		op_code_base = mem_get_addr (0);

		op_desc_t desc;

		word_t last_seg = 0xFFFF;
		word_t last_off_0 = 0xFFFF;
		word_t last_off_1;

		int flag_exit = 0;

		while (!flag_exit)
			{
			// Decode next instruction

			op_code_seg = seg_get (SEG_CS);
			op_code_off = reg16_get (REG_IP);

			// Code breakpoint test

			if (addr_seg_off (op_code_seg, op_code_off) == break_code_addr)
				{
				puts ("info: code breakpoint hit");
				flag_trace = 1;
				flag_prompt = 1;
				}

			int flag_exec = 1;

			// Optimize: no twice decoding of the same instruction
			// Example: REPeated or LOOP on itself

			if (op_code_seg != last_seg || op_code_off != last_off_0)
				{
				last_seg = op_code_seg;
				last_off_0 = op_code_off;

				memset (&desc, 0, sizeof desc);

				err = op_decode (&desc);
				if (err)
					{
					puts ("\nerror: unknown opcode");
					flag_trace = 1;
					flag_prompt = 1;
					flag_exec = 0;
					}

				// Suspicious operation on null opcodes

				if (op_code_null)
					{
					puts ("\nerror: suspicious null opcodes");

					op_code_null = 0;
					flag_trace = 1;
					flag_prompt = 1;
					flag_exec = 0;
					}

				last_off_1 = op_code_off;
				}
			else
				{
				op_code_off = last_off_1;
				}

			if (flag_trace)
				{
				// Print processor status

				putchar ('\n');
				regs_print ();
				putchar ('\n');

				printf ("%.4hX:%.4hX  ", seg_get (SEG_CS), reg16_get (REG_IP));
				print_column (op_code_str, 3 * OPCODE_MAX + 1);
				print_op (&desc);
				puts ("\n");
				}

			// User prompt

			if (flag_prompt)
				{
				// Get user command
				// Ugly but temporary

				char com [8];
				putchar ('>');
				char * res = fgets (com, 8, stdin);
				if (!res) break;

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
						break_code_addr = addr_seg_off (op_code_seg, op_code_off);
						flag_trace = 0;
						flag_prompt = 0;
						break;

					// Go (keep breakpoints)

					case 'g':
						flag_trace = 0;
						flag_prompt = 0;
						break;

					// Quit

					case 'q':
						flag_exec = 0;
						flag_exit = 1;
						break;

					// Interrupt (only timer for now)

					case 'i':
						flag_exec = 0;
						err = exec_int (0x08);  // timer 0 interrupt
						if (err) puts ("error: timer interrupt");
						break;

					}
				}

			// Execute operation

			if (flag_exec)
				{
				int trace_before = flag_get (FLAG_TF);

				reg16_set (REG_IP, op_code_off);

				err = op_exec (&desc);
				if (err)
					{
					puts (err < 0 ? "error: execute operation" : "warning: paused (HLT)");
					flag_trace = 1;
					flag_prompt = 1;
					if (err < 0) reg16_set (REG_IP, last_off_0);
					}
				else
					{
					// Repeat the operation if prefixed

					if (rep_active ())
						{
						reg16_set (REG_IP, last_off_0);
						}
					else
						{
						seg_reset ();
						}

					// Trace the operation if no prefix

					if (rep_none () && seg_none () && trace_before && flag_get (FLAG_TF))
						{
						err = exec_int (0x01);  // trace interrupt
						if (err)
							{
							puts ("fatal: trace interrupt");
							break;
							}
						}
					}
				}

			// Data breakpoint test

			if (_break_data_flag)
				{
				puts ("info: data breakpoint hit");
				_break_data_flag = 0;
				flag_trace = 1;
				flag_prompt = 1;
				}
			}

		break;
		}

	// Cleanup

	serial_term ();

	return exit_code;
	}
