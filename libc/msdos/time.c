
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

static long get_time(ah)
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	ax,[bx+2]
#endif
  mov	ah,al
  int	$21
  mov	ax,dx
  mov	dx,cx
#endasm
}

time_t time(timer)
time_t *timer;
{
  unsigned day,month,year;
  long rv;
  time_t t;

  rv = get_time(0x2C);
  rv >>= 8; t = (rv & 0xFF);
  rv >>= 8; t += (rv & 0xFF)*60;
  rv >>= 8; t += (rv & 0xFF)*3600;

  rv = get_time(0x2A);
  day = (rv & 0xFF);
  rv >>= 8; month = (rv & 0xFF) -1;
  rv >>= 8; year = (rv & 0xFFFF) - 1970;

  if (month <= 1 || year & 3)	/* if before Feb or not a leap year	*/
	day--;			/* don't add day for this leap year	*/
  day += mdays[month];		/* day in year				*/
  day += (year + 3) >> 2;	/* add a day for each leap year, but 	*/
  				/* don't worry about 2100 		*/
  t += ((year * 365L) + day) * SECSPERDAY;
  if (timer)
	*timer = t;
  return t;
}
