//-----------------------------------------------------------------------------
// MON86 - Host program
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/stat.h>
#include <sys/types.h>

//#include "../emu86/op-class.h"
//#include "../emu86/emu-proc.h"

#include "mon86-common.h"


//-----------------------------------------------------------------------------
// Target specifics
//-----------------------------------------------------------------------------

err_t read_char  (char_t * c)
	{
	assert (0);
	return E_OK;
	}

err_t write_char (char_t c)
	{
	assert (0);
	return E_OK;
	}

err_t read_string  (char_t * s, word_t * len)
	{
	assert (0);
	return E_OK;
	}

err_t write_string (char_t * s, word_t len)
	{
	assert (0);
	return E_OK;
	}


//-----------------------------------------------------------------------------
// Serial port operations
//-----------------------------------------------------------------------------

static int fd_serial = -1;


// Open serial port (as TTY)

static int serial_open (char * path)
	{
	int err;

	while (1)
		{
		if (fd_serial >= 0) close (fd_serial);

		fd_serial = open (path, O_RDWR | O_NOCTTY);
		if (fd_serial < 0)
			{
			perror ("open tty");
			err = -1;
			break;
			}

		// TTY setup

		struct termios tios;
		tcgetattr (fd_serial, &tios);
		cfsetspeed (&tios, B9600);
		tcsetattr (fd_serial, TCSANOW, &tios);

		err = 0;
		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
// Context operations
//-----------------------------------------------------------------------------

static context_t context_serial;
static context_t context_local;


// Synchronize contexts

static int context_synch ()
	{
	int err;

	while (1)
		{
		if (fd_serial < 0)
			{
			err = -1;
			break;
			}

		if (context_local.segment != context_serial.segment)
			{
			err = send_word (fd_serial, context_local.segment);
			if (err) break;
			err = send_segment (fd_serial);
			if (err) break;

			context_serial.segment = context_local.segment;
			}

		if (context_local.offset != context_serial.offset)
			{
			err = send_word (fd_serial, context_local.offset);
			if (err) break;
			err = send_offset (fd_serial);
			if (err) break;

			context_serial.offset = context_local.offset;
			}

		err = 0;
		break;
		}

	return err;
	}


// Initialize contexts

static int context_init ()
	{
	int err;

	while (1)
		{
		if (fd_serial < 0)
			{
			err = -1;
			break;
			}

		memset (&context_serial, 0, sizeof (context_t));
		memset (&context_local , 0, sizeof (context_t));

		err = send_word (fd_serial, context_serial.segment);
		if (err) break;
		err = send_segment (fd_serial);
		if (err) break;

		err = send_word (fd_serial, context_serial.offset);
		if (err) break;
		err = send_offset (fd_serial);
		if (err) break;

		err = 0;
		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
// Local file operations
//-----------------------------------------------------------------------------

static int fd_local = -1;


// Open local file

static int local_open (char * path)
	{
	int err;

	while (1)
		{
		if (fd_local >= 0) close (fd_local);

		fd_local = open (path, O_RDWR);
		if (fd_local < 0)
			{
			perror ("open local file");
			err = -1;
			break;
			}

		err = 0;
		break;
		}

	return err;
	}


static int read_to_file ()
	{
	int err;

	int fd;

	while (1)
		{
		if (fd_serial < 0)
			{
			err = -1;
			break;
			}

		fd = fd_local;
		if (fd < 0) fd = 1;  // default stdout

		err = context_synch ();
		if (err) break;

		while (context_local.count-- > 0)
			{
			err = send_command (fd_serial, 'R', 0);  // read memory
			if (err) break;

			word_t val;
			err = recv_value (fd_serial, &val);
			if (err) break;

			context_serial.offset++;

			// Write raw to local file

			int n = write (fd, &val, 1);
			if (n != 1)
				{
				err = -1;
				break;
				}
			}

		err = 0;
		break;
		}

	return err;
	}


static int write_from_file ()
	{
	int err;

	err = -1;
	return err;
	}


//-----------------------------------------------------------------------------
// Program main
//-----------------------------------------------------------------------------

int main (int argc, char * argv [])
	{
	int exit_code = 0;

	while (1)
		{
		int err;

		int interactive = 0;

		// Command line processing

		char opt;

		while (1)
			{
			opt = getopt (argc, argv, "t:s:o:l:f:rwxi");
			if (opt < 0 || opt == '?') break;

			switch (opt)
				{
				// Serial port

				case 't':
					err = serial_open (optarg);
					if (err) break;
					context_init ();
					break;

				// Segment

				case 's':
					if (sscanf (optarg, "%hx", &context_local.segment) != 1)
						{
						puts ("error: bad segment");
						err = -1;
						break;
						}

					printf ("info: segment %.4Xh\n", context_local.segment);
					err = 0;
					break;

				// Offset

				case 'o':
					if (sscanf (optarg, "%hx", &context_local.offset) != 1)
						{
						puts ("error: bad offset");
						err = -1;
						break;
						}

					printf ("info: offset %.4Xh\n", context_local.offset);
					err = 0;
					break;

				// Count

				case 'l':
					if (sscanf (optarg, "%hx", &context_local.count) != 1)
						{
						puts ("error: bad count");
						err = -1;
						break;
						}

					printf ("info: count %.4Xh\n", context_local.count);
					err = 0;
					break;

				// File path

				case 'f':
					err = local_open (optarg);
					if (err) break;
					printf ("info: file path %s\n", optarg);
					break;

				// Read memory to file

				case 'r':
					err = read_to_file ();
					break;

				// Write memory from file

				case 'w':
					err = write_from_file ();
					break;

				// Call procedure

				case 'x':
					err = call_proc ();
					break;

				// Interactive mode

				case 'i':
					interactive = 1;
					err = 0;
					break;

				}

			if (err) break;
			}

		if (err) break;

		// Check end of command line

		if (opt == '?' || optind != argc)
			{
			printf ("usage: %s [options]\n\n", argv [0]);
			puts ("  -t <tty>      serial port");
			puts ("  -s <segment>  (hex)");
			puts ("  -o <offset>   (hex)");
			puts ("  -l <count>    (hex)");
			puts ("  -f <path>     file path");
			puts ("  -r            read memory to file");
			puts ("  -w            write memory from file");
			puts ("  -p            call procedure");
			exit_code = 1;
			break;
			}

		// Main loop

		if (!interactive) break;

		context_t context_op;
		//op_desc_t op_desc;

		while (1)
			{
			// Print processor status

			//print_regs ();
			//print_char ('\n');

			//memset (&context_op, 0, sizeof (context_t));
			//context_op.segment = seg_get (SEG_CS);
			//context_op.offset = reg16_get (REG_IP);

			//memset (&op_desc, 0, sizeof (op_desc_t));

			//err = op_decode (&context_op, &op_desc);
			//if (err) break;

			//print_op (&context_op, &op_desc);
			//print_char ('\n');

			// Get user command

			print_char ('>');
			err = scan_context (&context_local);
			if (err == E_OK && context_local.length)
				{
				switch (context_local.token [0])
					{
					// Quit

					case 'Q':
						err = E_END;
						break;

					// Dump memory

					case 'M':
						if (context_local.length != 1)
							{
							err = E_LENGTH;
							break;
							}

						//print_mem (&context_local);
						break;

					default:
						err = E_VALUE;

					}
				}

			print_error (err);

			if (err == E_END) break;
			}

		break;
		}

	// Cleanup

	if (fd_serial >= 0) close (fd_serial);
	if (fd_local  >= 0) close (fd_local);

	return exit_code;
	}
