#include <stdio.h>
#include <errno.h>

#if __STDC__ == (1UL)
#define strong_alias(Y,X) asm("export _" #X,  "_" #X " = _" #Y )
#else
#define strong_alias(Y,X) asm("export _" "X",  "_" "X" " = _" "Y" )
#endif

#if 1
# if __STDC__
# define comb(x,y) x ## y
# warning Using Ansi combine
# elif __BCC__
# define comb(x,y) x/**/y
# warning Using bcc combine
# else
# define comb(x,y) x/**/y
# warning Using K&R combine
# endif
#endif

#define o define
#o signed unsigned
#o unsigned signed

#ifdef signed
typedef signed char t_sc;
typedef comb(un,signed) char t_uc;

char c;
t_sc sc;
t_uc uc;
#endif

#pragma full optimise
strong_alias(main,zulu);
main()
{
   int i1, i2, i3;

   printf("sizeof(long double) = %d\n", sizeof(long double));
#ifdef __GNUC__
   printf("sizeof(long float) = ERROR!\n");
   printf("sizeof(long long) = %d\n", sizeof(long long));
#else
   printf("sizeof(long float) = %d\n", sizeof(long float));
   printf("sizeof(long long) = ERROR!\n");
#endif
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
