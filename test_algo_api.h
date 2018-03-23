#ifndef __TEST_ALGO_API_T__
#define __TEST_ALGO_API_T__

#define DECISION_DATA_LEN 64

struct alg_info {
	void *enroll_ctx;
	unsigned char *pFeat;
	int m_ftsize;
	unsigned int max_enroll_cnt;
	int enroll_result;
	int percentage;
	unsigned int redundant_threshold;
	unsigned int enrolled_cnt;
	int image_qty;
	int matched_score;
	int matched_bioidx;
	int isLearningUpdate;
	int has_been_duplicate_check;
	int huawei_enroll_result;
	int identify_session;
};


void doTestAlgo();

#endif