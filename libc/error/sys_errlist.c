/* Copyright (C) 1996 Robert de Bath <robert@debath.thenet.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is a flash way of auto-initing the error array from an external file
 * I wouldn't be surprised tho if it's a lot better just to hard code the
 * error messages into the array.
 *
 * Of course the best of all is to use strerror().
 */

#if defined(__AS386_16__) || defined(__AS386_32__)
#define NR_ERRORS	128

extern char **__sys_errlist;
extern int __sys_nerr;

char *sys_errlist[NR_ERRORS];
int sys_nerr = NR_ERRORS;

#ifdef __AS386_16__
#asm
  loc	1		! Make sure the pointer is in the correct segment
auto_func:		! Label for bcc -M to work.
  .word	_init_vars	! Pointer to the autorun function
  .text			! So the function after is also in the correct seg.
#endasm
#else
#asm
  loc	1		! Make sure the pointer is in the correct segment
auto_func:		! Label for bcc -M to work.
  .long	_init_vars	! Pointer to the autorun function
  .text			! So the function after is also in the correct seg.
#endasm
#endif

static void init_vars()
{
   char inbuf[256];
   char errbuf[80];
   int i, cc, fd, err, len, bufoff=0;
   char * ptr;

   fd = open("/lib/liberror.txt", 0);
   if( fd < 0 ) return;

   for(i=0; i<NR_ERRORS; i++) sys_errlist[i] = "Unknown error";

   while( (cc=read(fd, inbuf, sizeof(inbuf))) > 0 )
   {
      for(i=0; i<cc; i++)
      {
         if( inbuf[i] == '\n' )
	 {
	    errbuf[bufoff] = '\0';
	    err = atoi(errbuf);
	    ptr = strchr(errbuf, ' ');
	    if( ptr && err > 0 && err < NR_ERRORS )
	    {
	       while(*ptr == ' ') ptr++;
	       len = strlen(ptr)+1;
	       sys_errlist[err] = (void*)sbrk(len);
	       if( (int)sys_errlist[err] == -1 )
	       {
	          sys_errlist[err] == "";
		  break;
	       }
	       memcpy(sys_errlist[err], ptr, len);
	    }
	    bufoff = 0;
	 }
	 else if( bufoff < sizeof(errbuf)-1 )
	    errbuf[bufoff++] = inbuf[i];
      }
   }
   close(fd);

   __sys_errlist = sys_errlist;
   __sys_nerr = sys_nerr = NR_ERRORS;
}

#endif /* __AS386_??__ */
