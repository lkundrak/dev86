
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

main(argc, argv)
int argc;
char ** argv;
{
   struct stat st;
   long exe_mtime = 0;
   int verbose = 0;
   int ar;

   if( strcmp(argv[1], "+") == 0 ) { verbose++; argc--; argv++; }

   if( stat(argv[1], &st) < 0 ) 
   {
      if( verbose ) printf("%s not found\n", argv[1]);
      exit(2);
   }
   exe_mtime = st.st_mtime;

   for(ar=2; ar<argc; ar++)
   {
      if( stat(argv[ar], &st) < 0 )
      {
          printf("ERROR: dependent %d not found\n", argv[ar]);
	  exit(3);
      }
      if( exe_mtime < st.st_mtime )
      {
          if( verbose ) printf("%s out of dat wrt %s \n", argv[1], argv[ar]);
	  exit(1);
      }
   }
   exit(0);
}
