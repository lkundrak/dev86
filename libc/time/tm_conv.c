
#include <time.h>

/* This is a translation from ALGOL in Collected Algorithms of CACM. */
/* Copied from Algorithm 199, Author: Robert G. Tantzen */

void
__tm_conv(tmbuf, timep, offset)
struct tm *tmbuf;
time_t *timep;
time_t offset;
{
static int   moffset[] =
   {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

   long s;
   long  j, d, m, y;

   offset += *timep;

   tmbuf->tm_isdst = 0;		/* Someone else can set this */

   j = offset / 86400L + 719469;
   s = offset % 86400L;

   if( s < 0 ) { s += 86400L; j--; }

   tmbuf->tm_sec = s % 60;
   tmbuf->tm_min = (s / 60) % 60;
   tmbuf->tm_hour = s / 3600;

   tmbuf->tm_wday = (j+2) % 7;

   /*
    * Julian date converter. Takes a julian date (the number of days since
    * some distant epoch or other), and fills tmbuf.
    */

   y = (4L * j - 1L) / 146097L;
   j = 4L * j - 1L - 146097L * y;
   d = j / 4L;
   j = (4L * d + 3L) / 1461L;
   d = 4L * d + 3L - 1461L * j;
   d = (d + 4L) / 4L;
   m = (5L * d - 3L) / 153L;
   d = 5L * d - 3 - 153L * m;
   d = (d + 5L) / 5L;
   y = 100L * y + j;
   if (m < 10)
      m += 2;
   else
   {
      m -= 10;
      ++y;
   }

   tmbuf->tm_year = y - 1900;
   tmbuf->tm_mon = m;
   tmbuf->tm_mday = d;

   tmbuf->tm_yday = d + moffset[m];
   if (m > 1 && ((y) % 4 == 0 && ((y) % 100 != 0 || (y) % 400 == 0)))
      tmbuf->tm_yday++;
}
