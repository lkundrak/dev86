#ifndef _BSD_TZFILE_H
#define _BSD_TZFILE_H

#define SECSPERDAY	(60*60*24)
#define DAYSPERNYEAR	365
#define DAYSPERLYEAR    366

#define isleap(y) (((y) % 4) == 0 && ((y) % 100) != 0 || ((y) % 400) == 0)

#endif /* _BSD_TZFILE_H */
