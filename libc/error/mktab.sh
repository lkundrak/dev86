#!/bin/sh -

awk '{
  e=$0; 
  sub("^[^ ]* ", "", e);
  sub(" [^ ]*$", "", e);
  n=0+$1;
  if (!(n in errlist))
     errlist[n] = e;
  if(n > maxerr) maxerr=n;
}
END{
   printf ("#define NR_ERRORS\t%d\n", maxerr+1);
   printf ("int sys_nerr = NR_ERRORS;\n");
   printf ("char *sys_errlist[NR_ERRORS] = {\n");

   for(i=0; i<=maxerr; i++) {
      if (errlist[i] == "")
         printf("   \"Error %d\"", i);
      else
         printf("   \"%s\"", errlist[i]);
      if (i != maxerr) printf(",\n"); else printf("\n");
   }
   printf ("};\n");
}' < liberror.txt > error_list.h
