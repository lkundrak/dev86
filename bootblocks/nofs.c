
#ifndef __STANDALONE__

#include <sys/types.h>
#include <sys/stat.h>
#include "readfs.h"

static int fd = -1;

open_file(fname)
char * fname;
{
   if( fd >= 0 ) close(fd);
   fd = open(fname, 0);
   if( fd >= 0 ) return 0;
   return -1;
}

rewind_file()
{
   if( fd == -1 ) return -1;
   lseek(fd, 0L, 0);
   return 0;
}

close_file()
{
   if( fd >= 0 ) close(fd);
   fd = -1;
}

long
file_length()
{
   struct stat st;
   if( fd == -1 ) return -1;
   if( fstat(fd, &st) < 0 ) return -1;

   return st.st_size;
}

read_block(buffer)
char * buffer;
{
   int rv;
   if( fd == -1 ) return -1;

   rv = read(fd, buffer, 1024);
   if( rv <= 0 ) return -1;
   if( rv < 1024 )
      memset(buffer+rv, '\0', 1024-rv);
   return 0;
}

#endif
