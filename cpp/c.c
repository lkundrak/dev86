#include <stdio.h>
#include <errno.h>

#if __STDC__
#define strong_alias(Y,X) asm("export _" #X,  "_" #X " = _" #Y )
#else
#define strong_alias(Y,X) asm("export _" "X",  "_" "X" " = _" "Y" )
#endif

#if __STDC__
#define comb(x,y) x##y
#warning Using Ansi combine
#elif __BCC__
#define comb(x,y) x/**/y
#warning Using bcc combine
#else
#define comb(x,y) x/**/y
#warning Using K&R combine
#endif

#define signed unsigned
#define unsigned signed

#ifdef signed
typedef signed char t_sc;
typedef comb(un,signed) char t_uc;
#endif

char c;
t_sc sc;
t_uc uc;

strong_alias(main,zulu);
main()
{
   int i1, i2, i3;

   printf("sizeof(long double) = %d\n", sizeof(long double));
   printf("sizeof(long float) = %d\n", sizeof(long float));
   printf("sizeof(double) = %d\n", sizeof(double));
   printf("sizeof(float) = %d\n", sizeof(float));

   c = -6;
   uc = -6;
   sc = -6;

   printf("%ld, ", (long)c);
   printf("%ld, ", (long)uc);
   printf("%ld\n", (long)sc);

   printf("%d, ", c);
   printf("%d, ", uc);
   printf("%d\n", sc);

   i1 = c; i2 = uc; i3 = sc;

   printf("%d, ", i1);
   printf("%d, ", i2);
   printf("%d\n", i3);

   i1 = (char) 200 + (char) 50;
   i2 = (t_uc) 200 + (t_uc) 50;
   i3 = (t_sc) 200 + (t_sc) 50;

   printf("%d, ", i1);
   printf("%d, ", i2);
   printf("%d\n", i3);

   c = 200; uc = 200; sc = 200;

   i1 = c  + (long) 50;
   i2 = uc + (long) 50;
   i3 = sc + (long) 50;

   printf("%d, ", i1);
   printf("%d, ", i2);
   printf("%d\n", i3);
}
