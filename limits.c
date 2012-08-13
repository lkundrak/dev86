

#if ~0==-1

#define UINT_MAX	(~0U)
#define INT_MAX		((int)((~0U)>>1))
#define INT_MIN		(~(int)((~0U)>>1))

#define ULONG_MAX	(~0UL)
#define LONG_MAX	((long)((~0UL)>>1))
#define LONG_MIN	(~(long)((~0UL)>>1))

#define USHRT_MAX	((unsigned short)-1)
#define SHRT_MAX	((int)(((unsigned short)-1)>>1))
#define SHRT_MIN	(~(int)(((unsigned short)-1)>>1))

#define UCHAR_MAX	((unsigned char)-1)

#if 255 == '\377'
#define CHAR_MAX	UCHAR_MAX
#define CHAR_MIN	0
#else
#define CHAR_MAX	((int)(((unsigned char)-1)>>1))
#define CHAR_MIN	(~(int)(((unsigned char)-1)>>1))
#endif

#endif

main()
{
   printf("UCHAR_MAX %d\n", UCHAR_MAX);
   printf("CHAR_MAX  %d\n", CHAR_MAX);
   printf("CHAR_MIN  %d\n", CHAR_MIN);

   if (USHRT_MAX == UINT_MAX && SHRT_MAX == INT_MAX && SHRT_MIN == INT_MIN)
      printf("SHORT     AS INT\n");
   else {
      printf("USHRT_MAX %u\n", USHRT_MAX);
      printf("SHRT_MAX  %d\n", SHRT_MAX);
      printf("SHRT_MIN  %d\n", SHRT_MIN);
   }

   printf("UINT_MAX  %u\n", UINT_MAX);
   printf("INT_MAX   %d\n", INT_MAX);
   printf("INT_MIN   %d\n", INT_MIN);

   if (ULONG_MAX == UINT_MAX && LONG_MAX == INT_MAX && LONG_MIN == INT_MIN)
      printf("LONG      AS INT\n");
   else {
      printf("ULONG_MAX %lu\n", ULONG_MAX);
      printf("LONG_MAX  %ld\n", LONG_MAX);
      printf("LONG_MIN  %ld\n", LONG_MIN);
   }

   if( sizeof(int) < 4 )     printf("SMALL_INTS\n");
   if( sizeof(char *) <= 2 ) printf("SMALL_MEMORY\n");
   if( sizeof(long) == 8 )   printf("LONG_8_BYTE\n");
   if( sizeof(int) != sizeof(char*) ) {
      if( sizeof(long) == sizeof(char*) )
	 printf("LONG_PTR_SIZE %d\n", sizeof(char*));
      else
	 printf("EVIL_PTR_SIZE %d\n", sizeof(char*));
   }
   if( sizeof(int) != sizeof(long) )
      printf("TRUE_LONGS %d\n", sizeof(long));
}
