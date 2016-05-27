
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>


// Common code

#include "pt1-common.c"


int main (int argc, char * argv [])
	{
	int err = 0;

	int f = -1;

	while (1)
		{
		if (argc != 4) break;

		word_t segment = 0;
		word_t offset = 0;
		word_t count = 0x100;

		sscanf (argv [2], "%hx:%hx", &segment, &offset);
		sscanf (argv [3], "%hx", &count);

		f = open (argv [1], O_RDWR | O_NOCTTY);
		if (f < 0)
			{
			perror ("open tty");
			err = -1;
			break;
			}

		_fi = f;
		_fo = f;

		// TTY setup

		struct termios tios;
		tcgetattr (f, &tios);
		//cfmakeraw (&tios);
		cfsetspeed (&tios, B9600);
		tcsetattr (f, TCSANOW, &tios);

		byte_t len;
		byte_t token [TOKEN_LEN_MAX];

		// Check header

		err = send_value (0);
		if (err)
			{
			perror ("send header");
			break;
			}

		err = recv_status ();
		if (err)
			{
			perror ("receive header");
			break;
			}

		// Set segment and offset

		err = send_value (segment);
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status segment");
			break;
			}

		err = send_command ('W', '7');  // DS register
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status W7");
			break;
			}

		err = send_value (offset);
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status offset");
			break;
			}

		err = send_command ('W', '4');  // SI register
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status W4");
			break;
			}

		while (count-- > 0)
			{
			err = send_command ('X', '0');  // read command
			if (err) break;

			word_t val;
			err = recv_value (&val);
			if (err) break;

			err = recv_status ();
			if (err)
				{
				perror ("receive status X0");
				break;
				}

			// Write raw to stdout

			int n = write (1, &val, 1);
			if (n != 1)
				{
				err = -1;
				break;
				}
			}

		break;
		}

	// Cleanup

	_fi = -1;
	_fo = -1;

	if (f)
		{
		close (f);
		f = -1;
		}

	return err;
	}
