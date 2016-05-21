
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>

#include "pcat-main.h"


int main (int argc, char * argv [])
	{
	while (1)
		{
		if (argc != 2) break;

		int f = open (argv [1], O_RDWR | O_NOCTTY);
		if (f < 0) break;

		char ci, co;
		char ei, eo;

		ei = eo = 0;  // EOF flags

		fd_set fdsr;

		int n;

		while (1)
			{
			FD_ZERO (&fdsr);

			if (!ei) FD_SET (0, &fdsr);
			if (!eo) FD_SET (f, &fdsr);

			int s = select (f + 1, &fdsr, NULL, NULL, NULL);
			if (s < 0) break;

			// Input pipe

			if (FD_ISSET (0, &fdsr))
				{
				n = read (0, &ci, 1);
				if (n != 1)
					{
					ei = 1;
					}
				else
					{
					if (ci == 0x0A) ci = 0x0D;
					n = write (f, &ci, 1);
					if (n != 1) break;
					}
				}

			// Output pipe

			if (FD_ISSET (f, &fdsr))
				{
				n = read (f, &co, 1);
				if (n != 1)
					{
					eo = 1;
					}
				else
					{
					n = write (1, &co, 1);
					if (n != 1) break;
					}
				}

			// Both EOF

			if (ei && eo) break;
			}

		close (f);
		}

	return 0;
	}
