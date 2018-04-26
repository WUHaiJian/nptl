#ifndef __TEST_ALGO_API_T__
#define __TEST_ALGO_API_T__

#define DECISION_DATA_LEN 64
#define ROOT_PATH "/sdcard/fingerprint"
#define MAX_LEN 256

#include <stdbool.h>
#include <dirent.h>

#include "FPMatchLib.h"

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

typedef enum{
	FP_LIB_OK,
	FP_LIB_WAIT_EVENT_FINGER_PRESENT,
	FP_LIB_CAPTURE_DONE,
	FP_LIB_ENABLE_EVENT_FINGER_PRESENT,
	FP_LIB_WAIT_TIME,
	FP_LIB_FINGER_PRESENT,
	FP_LIB_FINGER_LOST,
	FP_LIB_ERROR_TOO_FAST,
	FP_LIB_ERROR_TOO_SLOW,
	FP_LIB_ERROR_GENERAL,
	FP_LIB_ERROR_SENSOR,
	FP_LIB_ERROR_MEMORY,
	FP_LIB_ERROR_PARAMETER,
	FP_LIB_FAIL_LOW_QUALITY,
	FP_LIB_FAIL_IDENTIFY_START,
	FP_LIB_FAIL_IDENTIFY_IMAGE,
	
	
	FP_TA_ERROR_CODE_BASE = 0xFFFF6000,
	FP_TA_ERROR_TA_NOT_INIT,
	FP_TA_ERROR_STATE,
	FP_TA_ERROR_LIB_INIT_FAIL,
	FP_TA_ERROR_INIT_ALG_PPLIB_FAIL,
	FP_TA_ERROR_EROLL_EXCEED_MAX_FINGERPIRNTS,
	FP_TA_ERROR_EROLL_NOT_COMPLETED,
	FP_TA_ERROR_EROLL_GET_TEMPLATE_FAIL,
	FP_TA_ERROR_EROLL_PACK_TEMPLATE_FAIL,
	FP_TA_ERROR_EROLL_START_FAIL,
	FP_TA_ERROR_EROLL_ADD_IMAGE,
	
	FP_TA_DB_CODE_BASE = FP_TA_ERROR_CODE_BASE | 0x1000,
	FP_TA_ERROR_DB_FS_INIT_FAIL,
	FP_TA_ERROR_CREAT_GLOBAL_DB_FAIL,
	FP_TA_ERROR_CREAT_USER_DB_FAIL,
	FP_TA_ERROR_DB_SIZE_OVERFLOW,
	FP_TA_ERROR_DB_GET_FP_ID_ERROR,
	FP_TA_ERROR_DB_GET_FPSET_ID_ERROR,
	FP_TA_ERROR_DB_GET_SECURE_USER_ID_ERROR
}fp_lib_return_t;


#define IN 
#define OUT

#define WIDTH  (67)
#define HEIGHT (57)

//finger folder path : sdcard/fingerprint/0001/0/enroll/st

#define IMG_TYPE_STATUS(A) ((1<<A) & 0x0f)
enum IMG_TYPE {
	ENROLL = 0,
	VERIFY = 1,
	UNKNOWN
};

struct img_attr {
	struct img_pic_path ** pp_e_v_mgr;
	char path[MAX_LEN];
	int img_count;
	enum IMG_TYPE type;
	int fid;
};

struct img_pic_path {
	char path[MAX_LEN];
};

struct templ {
	char templ_path[MAX_LEN];
	unsigned char * tpl;
	int templ_size;
	int fid;
};

struct finger {
	struct img_attr ** pp_i_mgr;
	char path[MAX_LEN];
    int e_v_status; //under finger folder, have enroll and verify sub-folder
    int fid;
    struct person * of_p;	
};

struct person {
	struct finger ** pp_f_mgr;
	char path[MAX_LEN];
	int finger_count;
	//manager person's template
	struct templ ** pp_tmpl_mgr;
	int p_tpl_cnt; 
};

struct db {
	struct person ** ppmgr;
	struct templ ** pp_tmpl_mgr;
	int templ_count;
	int person_count;
	int total_finger_cnt;
};

struct  fa_fr_result {
	int fr;
	int fa;
	int total_fr;
	int total_fa;
};


int doTestEnroll();
void doTestAlgo();

int pre_enroll();

int identify_image_enroll(char * img, struct finger * f);
int do_enroll(struct alg_info* pSdk, char * img);
int update_enroll_templ(OUT char ** tpl, OUT int *len);
int finish_enroll(struct templ * template, IN char* tpl, IN int len);
int load_image(char *path, char *img, int *len);
int enroll_one_finger(struct finger * f);


int doTestVerify(struct templ * t);
int identify_image_verify(char * img, struct templ * t, int fid);
void finish_verify();
int pre_verify(struct templ * t);
void do_verify_uninit(void); 

int load_image_path(struct img_attr * pi);
int load_img_type(struct finger * pf);
int load_fingers(struct person * pp);
int load_db(char * path);
int traverse_dir_namelist(const char * path, struct dirent ***list, bool bfree);
void free_db();
void free_namelist(struct dirent **list, int itotalfile);
void free_image_path(struct img_attr * pi);

//int traverse_dir_namelist(const char * path);

#endif