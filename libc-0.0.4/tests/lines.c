
#include <string.h>
#include <fcntl.h>

char *
readline(fd)
{
static char linebuf[256];
   int cc;
   char * p;

   cc = read(fd, linebuf, sizeof(linebuf)-1);
   if( cc <= 0 ) return 0;
   p = strchr(linebuf, '\n');
   if( p == 0 ) p = linebuf+sizeof(linebuf)-1;
   else
   {
      p++; lseek(fd, (long)(p-linebuf)-cc, 1);
   }
   *p = 0;
   return linebuf;
}

main()
{
   int fd = open("/etc/passwd", O_RDONLY);
   char * p;

   if(fd<0) exit(1);
   
   while( p=readline(fd) )
   {
      write(1, ">>", 2);
      write(1, p, strlen(p));
   }
}
