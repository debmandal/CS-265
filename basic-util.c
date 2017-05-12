#include "lsmt.h"

void error_msg(int num, ...)
{
	int i;

	va_list valist;

	va_start(valist, num);
	for(i = 0; i < num; i++)
		fprintf(stderr, "%s", va_arg(valist, char*));

	va_end(valist);
	exit(1);

}
