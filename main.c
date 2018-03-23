#include <stdio.h>
#include "test_algo_api.h"
#include "log.h"

int main(int argc, const char* argv[])
{
	FILE *file = log_init();
	doTestAlgo();
	log_destroy(file);
	return 0;
}