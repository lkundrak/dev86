
#ifdef __ELKS__
#include <stdio.h>
#endif
#include "readfs.h"

int fs_type = 0;

open_file(fname)
char * fname;
{
#ifdef __ELKS__
   fprintf(stderr, "Open file %s\n", fname);
#endif
   if( fs_type ) close_file();

   if( tar_open_file(fname) >= 0 ) { fs_type = 1; return 0; }
   if( min_open_file(fname) >= 0 ) { fs_type = 2; return 0; }
   if( dos_open_file(fname) >= 0 ) { fs_type = 3; return 0; }
   return -1;
}

rewind_file()
{
#ifdef __ELKS__
   fprintf(stderr, "Rewind file (%d)\n", fs_type);
#endif
   switch(fs_type)
   {
   case 1: return tar_rewind_file();
   case 2: return min_rewind_file();
   case 3: return dos_rewind_file();
   }
   return -1;
}

close_file()
{
   int rv;
#ifdef __ELKS__
   fprintf(stderr, "Close file (%d)\n", fs_type);
#endif
   switch(fs_type)
   {
   case 1: rv = tar_close_file(); break;
   case 2: rv = min_close_file(); break;
   case 3: rv = dos_close_file(); break;
   }
   fs_type = 0;
   return -1;
}

long
file_length()
{
#ifdef __ELKS__
   fprintf(stderr, "File length (%d)\n", fs_type);
#endif
   switch(fs_type)
   {
   case 1: return tar_file_length();
   case 2: return min_file_length();
   case 3: return dos_file_length();
   }
   return -1;
}

read_block(buffer)
char * buffer;
{
#ifdef __ELKS__
   fprintf(stderr, "read block into (%d) (%d)\n", buffer, fs_type);
#endif
   switch(fs_type)
   {
   case 1: return tar_read_block(buffer);
   case 2: return min_read_block(buffer);
   case 3: return dos_read_block(buffer);
   }
   return -1;
}
