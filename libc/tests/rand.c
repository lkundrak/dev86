#include <stdio.h>

FILE * popen();

main()
{
   FILE * fd = popen("./hd", "w");
   int ch;

   srand(time((void*)0));

   for(ch=0; ch<256; ch++)
      putc(rand(), fd);

   pclose(fd);
}
