
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

		// TEST: load test code

		if (argc !=  2)
			{
			printf ("usage: %s <file>\n", argv [0]);
			exit_code = 1;
			break;
			}

		f = open (argv [1], O_RDONLY);
		if (f < 0)
			{
			print_string ("cannot open file\n");
			exit_code = 1;
			break;
			}

		off_t file_size = lseek (f, 0, SEEK_END);
		if (file_size <= 0)
			{
			print_string ("empty file\n");
			exit_code = 1;
			break;
			}

		printf ("file size=%lXh\n", file_size);
		if (file_size > MEM_MAX)
			{
			print_string ("file too big\n");
			exit_code = 1;
			break;
			}

		lseek (f, 0, SEEK_SET);

		byte_t * buf = mem_get_addr (0);
		off_t read_size = read (f, buf, file_size);
		if (read_size != file_size)
			{
			print_string ("incomplete file read\n");
			exit_code = 1;
			break;
			}

		op_code_base = buf;
		byte_t flag_u = 1;

		while (1)
			{
			op_code_cs = seg_get (SEG_CS);
			op_code_ip = reg16_get (REG_IP);

			if (flag_u)
				{
				printf ("%.4X:%.4X ", op_code_cs, op_code_ip);
				}

			op_desc_t desc;
			memset (&desc, 0, sizeof desc);
			int err = op_decode (&desc);

			if (flag_u)
				{
				print_column (op_code_str, 3 * OPCODE_MAX + 1);
				op_print (&desc);
				putchar ('\n');
				}

			if (err)
				{
				puts ("fatal: unknown opcode");
				break;
				}

			reg16_set (REG_IP, op_code_ip);

			op_hand_t hand = desc.op_hand;
			assert (hand);
			(* hand) (&desc);
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
