
#include <stdio.h>

char buf[256];

main()
{
   FILE * fd;
   fd = fopen("/etc/passwd", "r");

   while(fgets(buf, sizeof(buf), fd) != NULL)
   {
      printf(">>%s", buf);
   }
}
