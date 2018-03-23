#include "test_algo_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "EgisAlgorithmAPI.h"
#include "fp_algomodule.h"
#include "fp_types.h"
#include "plat_heap.h"


struct alg_info *pSdk = NULL;
char * g_decision_data = NULL;


int doTestInit()
{
	
	printf("%s\n", __func__);
	
	unsigned int ret = FP_LIB_OK;
	BOOL reset_flag = FALSE;
	int sensor_type = FP_ALGOAPI_MODE_UNKNOWN;
	if (pSdk == NULL) {
		pSdk = (struct alg_info *)malloc(sizeof(struct alg_info));
	}
	
	memset(pSdk, 0, sizeof(struct alg_info));

	printf("GIT_SHA1 = UNKNOWN\n");
	printf("FP TA build at %s %s\n", __DATE__ ,__TIME__);


#ifdef __ET516__
	sensor_type = FP_ALGOAPI_MODE_EGIS_ET516;
#elif __ET538__
	sensor_type = FP_ALGOAPI_MODE_EGIS_ET538;
#elif __ET512__
	sensor_type = FP_ALGOAPI_MODE_EGIS_ET512;
#else
	ret = FP_LIB_ERROR_GENERAL;
	goto exit;
#endif

	if (g_decision_data == NULL)
	{
		g_decision_data = malloc(DECISION_DATA_LEN);
		memset(g_decision_data, 0, DECISION_DATA_LEN);
        g_decision_data[0] = 0x11;
        g_decision_data[1] = 0x12;
        g_decision_data[2] = 0x13;
        g_decision_data[3] = 0x14;		
		reset_flag = TRUE;
		printf("reset_flag = %x\n", reset_flag);
	}

	printf("sensor type: %d\n", sensor_type);
	ret = algorithm_initialization_by_sensor(g_decision_data, DECISION_DATA_LEN, sensor_type, reset_flag);
	if (ret != FP_OK)
	{
		printf("algorithm_initialization_by_sensor, algorithm_initialization fail ,ret = %d\n", ret);
		return FP_LIB_ERROR_GENERAL;
	}

	//printf("egisalg:%s - %s",EGISLIB_VERSION, EGISLIB_PATCH);

exit:
	//convert_retval(&ret);
	printf("egis_fp_initAlgAndPPLib ret = %d\n", ret);

	return ret;
}

int doTestDestory()
{
	char out_data[DECISION_DATA_LEN];

	if (pSdk->pFeat)
	{
		printf("egis_fp_deInitAlg freeing pFeat\n");
		free(pSdk->pFeat);
		pSdk->pFeat = NULL;
	}
	if (pSdk->enroll_ctx != NULL)
	{
		printf("calling enroll_uninit\n");
		//enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}

	algorithm_uninitialization(out_data, DECISION_DATA_LEN);
	{
		int i;
		for (i = 0; i < 5; i++)
			printf("egis_alg_deinit %d 0x%x\n", i, out_data[i]);
	}

	if (g_decision_data != NULL)
		memcpy(g_decision_data, out_data, DECISION_DATA_LEN);	
	/*
	 *	To keep decision data in TZApp, Since Huawei doesn't provide api to storage sensor/algo data.
	 *	Don't free g_decision_data here. 
	 */
	
	free(g_decision_data);
	free(pSdk);
	return 0;
}

void doTestGetAlgoVersion()
{
	ALGO_API_INFO algo_api_version;
	char version[1024] = {0};
	algorithm_do_other(FP_ALGOAPI_GET_VERSION, NULL, (BYTE*)&algo_api_version);
	sprintf(version, "%s, %s\n", algo_api_version.algo_api_version, algo_api_version.matchlib_version);
	printf("algorithm version : %s\n", version);
}

void doTestAlgo()
{
	printf("%s\n", __func__);
	doTestInit();
	doTestGetAlgoVersion();
	doTestDestory();
	
}


