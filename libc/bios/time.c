/*
 * Time functions for standalone mode, on the first call to time() it tries
 * to query the RTC, on success it sets it's own day counter an the PC's
 * ticks since midnight clock. From the second call on it uses the day
 * counter and ticks clock.
 *
 * NOTES:
 *    DOS's midnight bug is here too, it's actually in the bios.
 *    If there's no RTC the clock will fall to Jan 3 1970.
 */

#include <time.h>

static int mdays[13] = { 0,31,31+28,31+28+31,31+28+31+30,
	31+28+31+30+31,31+28+31+30+31+30,31+28+31+30+31+30+31,
	31+28+31+30+31+30+31+31,31+28+31+30+31+30+31+31+30,
	31+28+31+30+31+30+31+31+30+31,31+28+31+30+31+30+31+31+30+31+30,
	365 };

#define SECSPERHOUR	(60*60)
#define SECSPERDAY	(SECSPERHOUR*24L)

/****************************************
 * Return the number of seconds that have elapsed since the start
 * of 1970.
 * Input:
 *	timer	pointer to where to store result (or NULL)
 * Output:
 *	*timer = result (unless timer == NULL)
 * Returns:
 *	time
 */

static int xt_days = 0, ax_val, huns = 0;

static long get_time(ah)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  mov	ah,al
  int	$1A
  jnc	intok
  mov	cx,#-1
  mov	dx,cx
intok:
  mov	[_ax_val],ax
  mov	ax,dx
  mov	dx,cx
#endasm
}

static int
unbcd(bcdval)
int bcdval;
{
   return (bcdval&0xF) + 10*((bcdval>>4)&0xF);
}

time_t time(timer)
time_t *timer;
{
  unsigned day,month,year;
  long rv;
  time_t t;

  if( xt_days ) goto XT_time;

  rv = get_time(0x02);
  if(rv == -1) goto XT_time;
          huns = unbcd(rv & 0xFF);	/* Save for stime */
  rv >>= 8; t =  unbcd(rv & 0xFF);
  rv >>= 8; t += unbcd(rv & 0xFF)*60;
  rv >>= 8; t += unbcd(rv & 0xFF)*3600;

  rv = get_time(0x04);
  if(rv == -1) goto XT_time;
            day   = unbcd(rv & 0xFF);
  rv >>= 8; month = unbcd(rv & 0xFF) -1;
  rv >>= 8; year  = unbcd(rv & 0xFF);
  rv >>= 8; year += unbcd(rv & 0xFF)*100;
            year -= 1970;
  if( year < 1950 && year >= 1900 )
     year += 100;		/* Century is not updated on RTC */

  if (month <= 1 || year & 3)	/* if before Feb or not a leap year	*/
	day--;			/* don't add day for leap year		*/
  day += mdays[month];		/* day in year				*/
  day += (year + 3) >> 2;	/* add a day for each leap year		*/
  t += ((year * 365L) + day) * SECSPERDAY;

  stime(t);

  if(0)
  {
XT_time:
     rv = get_time(0);
     xt_days += (ax_val&0xFF);
     rv = xt_days*SECSPERDAY + rv * 1080 / 19663;
  }

  if (timer)
	*timer = t;
  return t;
}

static long ticks;

stime(timer)
time_t timer;
{
   xt_days = (timer/SECSPERDAY);

   ticks = ((timer%SECSPERDAY) * 19663L + huns*196) / 1080;
   huns = 0;

#asm
   mov	cx,[_ticks+2]
   mov	dx,[_ticks]
   mov	ah,#1
   int	$1A
#endasm

   return 0;
}
