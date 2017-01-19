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

#include "mon86-common.h"


//-----------------------------------------------------------------------------
// Generic operations
//-----------------------------------------------------------------------------

static int _context_fd = -1;
static context_t _context;


err_t recv_char (char_t * c)
	{
	ssize_t n = read (_context_fd, c, 1);
	return (n == 1) ? E_OK : E_END;
	}

err_t send_char (char_t c)
	{
	ssize_t n = write (_context_fd, &c, 1);
	return (n == 1) ? E_OK : E_END;
	}


err_t send_string (char_t * s, word_t len)
	{
	ssize_t n = write (_context_fd, s, len);
	return (n == len) ? E_OK : E_END;
	}


//-----------------------------------------------------------------------------
// Serial port operations
//-----------------------------------------------------------------------------

static int _serial_fd = -1;


// Open serial port (as TTY)

static int serial_open (char * path)
	{
	int err;

	while (1)
		{
		if (_serial_fd >= 0) close (_serial_fd);

		_serial_fd = open (path, O_RDWR | O_NOCTTY);
		if (_serial_fd < 0)
			{
			perror ("open tty");
			err = -1;
			break;
			}

		// TTY setup

		struct termios tios;
		tcgetattr (_serial_fd, &tios);
		cfsetspeed (&tios, B9600);
		tcsetattr (_serial_fd, TCSANOW, &tios);

		err = 0;
		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
// Local file operations
//-----------------------------------------------------------------------------

static int _file_fd = -1;


// Open local file

static int file_open (char * path)
	{
	int err;

	while (1)
		{
		if (_file_fd >= 0) close (_file_fd);

		_file_fd = open (path, O_RDWR);
		if (_file_fd < 0)
			{
			perror ("open file");
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

	while (1)
		{
		int fd = _file_fd;
		if (fd < 0) fd = 1;  // default stdout

		_context_fd = _serial_fd;

		err = send_context (&_context);
		if (err) break;

		while (_context.count-- > 0)
			{
			err = send_command ('R', 0);  // read memory
			if (err) break;

			word_t val;
			err = recv_word (&val);
			if (err) break;

			err = recv_error ();
			if (err) break;

			_context.offset++;

			// Write raw to local file

			int n = write (fd, &val, 1);
			if (n != 1)
				{
				err = E_END;
				break;
				}
			}

		break;
		}

	return err;
	}


static int write_from_file ()
	{
	int err;

	err = E_END;
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
					break;

				// Segment

				case 's':
					if (sscanf (optarg, "%hx", &_context.segment) != 1)
						{
						puts ("error: bad segment");
						err = -1;
						break;
						}

					printf ("info: segment %.4Xh\n", _context.segment);
					err = 0;
					break;

				// Offset

				case 'o':
					if (sscanf (optarg, "%hx", &_context.offset) != 1)
						{
						puts ("error: bad offset");
						err = -1;
						break;
						}

					printf ("info: offset %.4Xh\n", _context.offset);
					err = 0;
					break;

				// Count

				case 'l':
					if (sscanf (optarg, "%hx", &_context.count) != 1)
						{
						puts ("error: bad count");
						err = -1;
						break;
						}

					printf ("info: count %.4Xh\n", _context.count);
					err = 0;
					break;

				// File path

				case 'f':
					err = file_open (optarg);
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
					//err = call_proc ();
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

			putchar ('>');

			_context_fd = 0; // stdin

			err = recv_context (&_context);
			if (err == E_OK && _context.length && ! _context.done)
				{
				switch (_context.token [0])
					{
					// Quit

					case 'Q':
						err = E_END;
						break;

					}
				}

			if (err == E_OK && ! _context.done) err = E_VALUE;

			_context_fd = 2;  // stderr

			send_error (err);

			if (err == E_END) break;
			}

		break;
		}

	// Cleanup

	if (_serial_fd >= 0) close (_serial_fd);
	if (_file_fd  >= 0)  close (_file_fd);

	return exit_code;
	}
