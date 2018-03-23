#include <stdio.h>
#include <stdlib.h>
#define MAX_BUFLEN 1024

static FILE* file = NULL;
FILE* log_init(void)
{
	char date[256] = {0};
	sprintf(date, "log_algo_%s_%s.txt", __DATE__, __TIME__);
	printf("%s\n", date);
	file = fopen(date, "w+");
	return file;
}

void log_destroy(FILE* file)
{
	if (file != NULL) 
	{
		fclose(file);
	}
}

void Log(const char* format, ...)
{
	char buffer[MAX_BUFLEN];
	if (format == NULL) return;
	
	va_list vl;
	va_start(vl, format);
	vsnprintf(buffer, MAX_BUFLEN, format, vl);
	va_end(vl);
	
	fprintf(file, "%s\n", buffer);	
}
