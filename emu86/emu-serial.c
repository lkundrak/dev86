
#define _XOPEN_SOURCE  // for ptsname()
#define _GNU_SOURCE  // for getpt()

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "emu-serial.h"


static int _ptm = -1;


void serial_send (byte_t c)
	{
	if (_ptm >= 0)
		{
		int n = write (_ptm, &c, 1);
		if (n != 1)
			{
			perror ("warning: cannot write to PTM:");  // TODO: propagate error
			}
		}
	}


byte_t serial_recv ()
	{
	byte_t c = 0xFF;

	if (_ptm >= 0)
		{
		int n = read (_ptm, &c, 1);
		if (n != 1)
			{
			perror ("warning: cannot read from PTM:");  // TODO: propagate error
			}
		}

	return c;
	}


byte_t serial_poll ()
	{
	fd_set fdsr;
	FD_ZERO (&fdsr);
	FD_SET (_ptm, &fdsr);
	struct timeval tv = { 0L, 0L };  // immediate
	int s = select (_ptm + 1, &fdsr, NULL, NULL, &tv);
	assert (s >= 0);
	if (FD_ISSET (_ptm, &fdsr)) return 1;
	return 0;
	}


void serial_init ()
	{
	while (1)
		{
		// Create pseudo terminal for serial emulation

		_ptm = getpt ();
		if (_ptm < 0)
			{
			perror ("warning: cannot create PTM:");
			break;
			}

		// The following functions return -1 on success !?!
		grantpt (_ptm);
		unlockpt (_ptm);

		// Set slave in raw mode (avoid echo and other cooking)

		struct termios tios;
		tcgetattr (_ptm, &tios);
		cfmakeraw (&tios);
		tcsetattr (_ptm, TCSANOW, &tios);

		char * path = ptsname (_ptm);
		printf ("info: PTS for serial emulation: %s\n", path);

		int f = open ("emu86.pts", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		write (f, path, strlen (path));
		close (f);

		break;
		}
	}


void serial_term ()
	{
	unlink ("emu86.pts");

	// Close pseudo terminal

	if (_ptm >= 0)
		{
		close (_ptm);
		_ptm = -1;
		}
	}
