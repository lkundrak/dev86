#include	<fcntl.h>
#include	<a.out.h>

int verbose = 0;

void size(filename)
	char	*filename;
{
	int	f;
	struct exec	ex;
	long	total;
	int	cc;

	if ((f = open(filename, O_RDONLY)) < 0 )
	{
		perror(filename);
		return;
	}
	cc = read(f, &ex, sizeof(ex));

	if (cc == sizeof(ex) && !BADMAG(ex))
	{
		total = ex.a_text + ex.a_data + ex.a_bss;
		if( verbose )
		{
			printf("Text segment of %s = %5ld (0x%lx)\n",
				filename, ex.a_text, ex.a_text);
			printf("Init data of %s    = %5ld (0x%lx)\n",
				filename, ex.a_data, ex.a_data);
			printf("Uninit data of %s  = %5ld (0x%lx)\n",
				filename, ex.a_bss, ex.a_bss);
			printf("Data segment of %s = %5ld (0x%lx)\n",
				filename, ex.a_total, ex.a_total);
			printf("Minimum size of %s = %5ld (0x%lx)\n",
				filename, total, total);

			total = ex.a_total;
			if( ex.a_flags & A_SEP )
				total += ex.a_text;
			printf("Maximum size of %s = %5ld (0x%lx)\n",
				filename, total, total);
		}
		else
			printf("%-ld\t%-ld\t%-ld\t%-ld\t%-lx\t%s\n",
				ex.a_text, ex.a_data, ex.a_bss, total, total,	
				filename);
		}
	else if( cc > 16 && memcmp(&ex, "\243\206\001\000*", 5) == 0 )
	{	/* *.o file */
		total = ((unsigned char*)&ex)[9] +
		        ((unsigned char*)&ex)[10] * 256;
		if( verbose )
			printf("Size of object %s  = %5ld (0x%lx)\n",
				filename, total, total);
		else
			printf("\t\t\t%-ld\t%-lx\t%s\n",
				total, total, filename);
	}
	else
		printf("%s: Not an a.out file\n", filename);
	close(f);
}

int main(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc > 1 && strcmp(argv[1], "-v") == 0 )
	{
	   verbose++;
	   argc--, argv++;
	}
	if (argc < 2)
	{
		printf("Usage: %s [-v] file\n", argv[0]);
		exit(1);
	}
	if(!verbose)
		printf("text\tdata\tbss\tdec\thex\tfilename\n");
	for (--argc, ++argv; argc > 0; --argc, ++argv)
		size(*argv);
	exit(0);
}
