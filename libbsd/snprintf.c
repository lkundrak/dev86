/* snprintf.c - emulate BSD snprintf with sprintf - rick sladkey */

#include <stdio.h>
#include <stdarg.h>

int snprintf(char *s, int len, char *format, ...)
{
	va_list args;
	int result;

	va_start(args, format);
	result = vsprintf(s, format, args);
	va_end(args);
	return result;
}
