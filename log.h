#ifndef __LOG_T__
#define __LOG_T__
#include <stdio.h>
#include <stdlib.h>

FILE* log_init(void);
void log_destroy(FILE* file);
void Log(const char* format, ...);

#endif