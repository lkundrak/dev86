
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>


int main (int argc, char * argv [])
	{
	int exit_code = 0;

	while (1)
		{
		unsigned long time_arg_1 = 0L;
		unsigned long time_arg_2 = 0L;

		char opt = '?';

		while (1)
			{
			opt = getopt (argc, argv, "d:t:");
			if (opt < 0 || opt == '?') break;

			switch (opt)
				{
				case 'd':  // delay between input characters
					if (sscanf (optarg, "%lu", &time_arg_1) != 1)
						{
						puts ("error: bad -d value");
						}

					break;

				case 't':  // timeout before pipe closure
					if (sscanf (optarg, "%lu", &time_arg_2) != 1)
						{
						puts ("error: bad -t value");
						}

					break;

				}
			}

		if (opt == '?' || optind != argc - 1)
			{
			printf ("usage: %s [options] <PTY device>\n\n", argv [0]);
			puts ("  -d <time>  delay between input characters");
			puts ("  -t <time>  timeout before pipe closure");
			puts ("  <time> in milliseconds");

			exit_code = 1;
			break;
			}

		struct timeval timeval_1 = { 0L, 0L };  // immediate
		struct timeval timeval_2 = { 0L, 0L };  // immediate

		if (time_arg_1)
			{
			timeval_1.tv_sec = time_arg_1 / 1000L;
			timeval_1.tv_usec = (time_arg_1 % 1000L) * 1000L;
			}

		if (time_arg_2)
			{
			timeval_2.tv_sec = time_arg_2 / 1000L;
			timeval_2.tv_usec = (time_arg_2 % 1000L) * 1000L;
			}

		int f = open (argv [optind], O_RDWR | O_NOCTTY);
		if (f < 0)
			{
			perror ("open()");
			break;
			}

		int state = 0;

		fd_set fdsr;

		while (1)
			{
			FD_ZERO (&fdsr);

			struct timeval * tvi = NULL;
			struct timeval tvo;

			switch (state)
				{
				case 0: FD_SET (0, &fdsr); break;
				case 1: tvi = &timeval_1; break;
				case 2: tvi = &timeval_2; break;
				default: assert (0);
				}

			FD_SET (f, &fdsr);

			if (tvi)
				{
				memcpy (&tvo, tvi, sizeof (struct timeval));
				tvi = &tvo;
				}

			int s = select (f + 1, &fdsr, NULL, NULL, tvi);
			if (s < 0)
				{
				perror ("select()");
				break;
				}

			// Empty pipe has high priority

			if (FD_ISSET (f, &fdsr))
				{
				char c;

				int n = read (f, &c, 1);
				if (n != 1) break;

				n = write (1, &c, 1);
				if (n != 1) break;

				continue;
				}

			// Fill pipe has lower priority

			if (state == 0 && FD_ISSET (0, &fdsr))
				{
				char c;

				int n = read (0, &c, 1);
				if (n != 1)
					{
					if (time_arg_2)
						{
						state = 2;  // piping end
						continue;
						}

					break;
					}

				if (c == 0x0A) c = 0x0D;  // EOL -> CR
				n = write (f, &c, 1);
				if (n != 1) break;

				if (time_arg_1)
					{
					state = 1;  // piping pause
					}

				continue;
				}

			// Select timeout

			if (state == 2) break;

			state = 0;  // piping run
			}

		close (f);
		break;
		}

	return exit_code;
	}
