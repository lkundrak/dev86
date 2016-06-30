
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>


// Common code

#include "pt1-common.c"


int main (int argc, char * argv [])
	{
	int err = -1;

	int f = -1;

	while (1)
		{
		if (argc != 3) break;

		word_t segment = 0;
		word_t offset = 0;

		sscanf (argv [2], "%hx:%hx", &segment, &offset);

		f = open (argv [1], O_RDWR | O_NOCTTY);
		if (f < 0)
			{
			perror ("open tty");
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
			perror ("receive status header");
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

		err = send_command ('W', 'B');  // CS register
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status WB");
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

		err = send_command ('W', 'A');  // IP register
		if (err) break;
		err = recv_status ();
		if (err)
			{
			perror ("receive status WA");
			break;
			}

		err = send_command ('X', '2');  // execute command
		if (err) break;

		/*
		err = recv_status ();
		if (err)
			{
			perror ("receive status X2");
			break;
			}
		*/

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
