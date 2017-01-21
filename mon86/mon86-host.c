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

#include <linux/limits.h>

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
static char _file_path [PATH_MAX] = { '\0' };


// Open local file

static int file_open (int flags)
	{
	int err;

	while (1)
		{
		if (_file_fd >= 0) close (_file_fd);

		_file_fd = open (_file_path, flags);
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
		if (_file_path [0] != '\0')
			{
			err = file_open (O_WRONLY | O_CREAT);
			if (err) break;
			}

		int fd = _file_fd;
		if (fd < 0) fd = 1;  // default stdout

		_context_fd = _serial_fd;

		err = send_context (&_context);
		if (err)
			{
			perror ("send context");
			break;
			}

		while (_context.count-- > 0)
			{
			err = send_string ("R ", 2);  // read memory
			if (err) break;

			word_t val;
			err = recv_word (&val);
			if (err) break;

			err = recv_status ();
			if (err) break;

			_context.offset++;

			// Write raw to file

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

	while (1)
		{
		if (_file_path [0] != '\0')
			{
			err = file_open (O_RDONLY);
			if (err) break;
			}

		int fd = _file_fd;
		if (fd < 0) fd = 0;  // default stdin

		_context_fd = _serial_fd;

		err = send_context (&_context);
		if (err)
			{
			perror ("send context");
			break;
			}

		while (1)
			{
			// Read raw from file

			word_t val;
			int n = read (fd, &val, 1);
			if (n != 1)
				{
				err = E_END;
				break;
				}

			err = send_word (val);
			if (err) break;

			err = recv_status ();
			if (err) break;

			err = send_string ("W ", 2);  // write memory
			if (err) break;

			err = recv_status ();
			if (err) break;

			_context.offset++;
			}

		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
// Other commands
//-----------------------------------------------------------------------------

static int call_proc ()
	{
	int err;

	while (1)
		{
		_context_fd = _serial_fd;

		err = send_context (&_context);
		if (err)
			{
			perror ("send context");
			break;
			}

		err = send_string ("P ", 2);  // call procedure
		if (err) break;

		err = recv_status ();
		if (err)
			{
			char str [128];
			sprintf (str, "call procedure: err=%u\n", err);
			write (2, str, strlen (str));
			break;
			}

		break;
		}

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
		int err = 0;

		// Command line processing

		char opt = '?';

		memset (&_context, 0, sizeof (context_t));

		while (1)
			{
			opt = getopt (argc, argv, "t:s:o:l:f:rwx");
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
						perror ("bad segment");
						err = -1;
						}

					break;

				// Offset

				case 'o':
					if (sscanf (optarg, "%hx", &_context.offset) != 1)
						{
						perror ("bad offset");
						err = -1;
						}

					break;

				// Count

				case 'l':
					if (sscanf (optarg, "%hx", &_context.count) != 1)
						{
						perror ("bad count");
						err = -1;
						break;
						}

					break;

				// File path

				case 'f':
					strcpy (_file_path, optarg);
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

				}

			if (err) break;
			}

		if (err)
			{
			exit_code = 1;
			break;
			}

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
			puts ("  -x            call procedure");

			exit_code = 1;
			break;
			}

		break;
		}

	// Cleanup

	if (_serial_fd >= 0) close (_serial_fd);
	if (_file_fd  >= 0)  close (_file_fd);

	return exit_code;
	}
