
char hex[] = "0123456789ABCDEF";

char buf[20];
main(argc, argv, envp)
int argc;
char ** argv;
char ** envp;
{
   int i,j; char *p; char * str;
   int * arg = &argc;

   for(j=0; j<8; j++)
   {
      phex(arg);
      putstr(":");
      for(i=0; i<8; i++)
      {
         putstr(" ");
         phex(*arg++);
      }
      putstr("\n");
   }

#if 0
   str = alloca(sizeof(hex)+2);
   putstr("Alloca = ");
   phex(&str);
   putstr(",");
   phex(str);
   putstr("\n");
#endif

   p = (char*) &argc;

   putstr("ARGC="); phex(argc); putstr("\n");
   putstr("ARGV="); phex(argv); putstr("\n");
   putstr("ENVP="); phex(envp); putstr("\n");
   for(i=0; i<argc; i++)
   {
      phex(argv[i]);
      putstr(":");
      putstr(argv[i]);
      putstr("\n");
   }
   putstr("ENV=>\n");
   for(; *envp; envp++)
   {
      phex(envp);
      putstr(":");
      phex(*envp);
      putstr(":");
      putstr(*envp);
      putstr("\n");
   }
}

phex(val)
{
   int i;
   printf("%04x", val);
}

putstr(str)
{
   printf("%s", str);
}

#if 0
int global_var_that_needs_init = 0x201;

#asm
  loc	1		! Make sure the pointer is in the correct segment
auto_func:		! Label for bcc -M to work.
  .word	_init_vars	! Pointer to the autorun function
  .text			! So the function after is also in the correct seg.
#endasm

static void init_vars()
{
   global_var_that_needs_init = getuid();
}
#endif
