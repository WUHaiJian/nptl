#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>

#define MAX_BUFLEN 1024
#define LOG_PATH "/system/bin/egis_log/"
static FILE* file = NULL;

bool isExistFolder(const char * path);
FILE* log_init(void)
{
	
	if (!isExistFolder(LOG_PATH))
	{
		printf("log folder not exist!Will Create it...\n");
		mkdir(LOG_PATH, 0777);
	}
	
	char date[256] = {0};
	time_t now;
	struct tm* time_now;
	time(&now);
	time_now = localtime(&now);
	printf("Local time is %s\n", asctime(time_now));
	
	sprintf(date, LOG_PATH"log_algo_%04d%02d%02d_%02d%02d.txt", 
	               time_now->tm_year + 1900, \
	               time_now->tm_mon + 1, \
	               time_now->tm_mday, \
				   time_now->tm_hour, \
				   time_now->tm_min);
												   
	printf("%s\n", date);
	file = fopen(date, "w+");
	if (file == NULL)
	{
		printf("open egis_log fail!\n");
	}
	return file;
}

void log_destroy(FILE* file)
{
	if (file != NULL) 
	{
		fflush(file);
		fclose(file);
	}
}

void Log(const char* format, ...)
{
	char raw_buf[MAX_BUFLEN];
	if (format == NULL) return;
	
	va_list vl;
	va_start(vl, format);
	vsnprintf(raw_buf, MAX_BUFLEN, format, vl);
	va_end(vl);
	char t_buf[MAX_BUFLEN] = {0};

	struct tm* t_now;
#if 0	
	time_t t;
	time(&t);
	t_now = localtime(&t);
#else	
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday(&tv, &tz);
	t_now = localtime(&tv.tv_sec);
#endif	
	
	sprintf(t_buf, "%02d-%02d %02d:%02d:%02d.%06ld  %s", \
	                t_now->tm_mon + 1, \
					t_now->tm_mday, \
					t_now->tm_hour, \
					t_now->tm_min, \
					t_now->tm_sec, \
					tv.tv_usec, \
					raw_buf);
					
					
	fprintf(file, "%s\n", t_buf);
	fflush(file);
    printf("%s\n", t_buf);	
}

bool isExistFolder(const char * path)
{
	struct stat s_buf;
	stat(path, &s_buf);
	if (S_ISDIR(s_buf.st_mode))
		return true;
	return false;
}  
