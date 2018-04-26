#include "test_algo_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "EgisAlgorithmAPI.h"
//#include "fp_algomodule.h"
//#include "fp_types.h"
#include "plat_heap.h"
#include <sys/stat.h>
#include <fcntl.h>


static struct alg_info g_sdk = {0};
static struct alg_info *pSdk = &g_sdk;
static struct fa_fr_result g_far_frr = {0};

unsigned char * g_decision_data = NULL;
struct db g_db = {0};

int doTestInit()
{
	
	//printf("%s\n", __func__);
	Log("%s\n", __func__);
	unsigned int ret = FP_LIB_OK;
	BOOL reset_flag = FALSE;
	int sensor_type = FP_ALGOAPI_MODE_UNKNOWN;
	if (pSdk == NULL) {
		pSdk = (struct alg_info *)malloc(sizeof(struct alg_info));
	}
	
	memset(pSdk, 0, sizeof(struct alg_info));
	//printf("FP TA build at %s %s\n", __DATE__ ,__TIME__);
    Log("FP TA build at %s %s", __DATE__ ,__TIME__);
#ifdef __ET538__
	sensor_type = FP_ALGOAPI_MODE_EGIS_ET538;
#elif __ET512__
	sensor_type = FP_ALGOAPI_MODE_EGIS_ET512;
#else
	ret = FP_LIB_ERROR_GENERAL;
	goto exit;
#endif

	if (g_decision_data == NULL)
	{
		g_decision_data = (unsigned char *)malloc(DECISION_DATA_LEN);
		memset(g_decision_data, 0, DECISION_DATA_LEN);
        g_decision_data[0] = 0x11;
        g_decision_data[1] = 0x12;
        g_decision_data[2] = 0x13;
        g_decision_data[3] = 0x14;		
		reset_flag = TRUE;
		Log("reset_flag = %x", reset_flag);
	}

	Log("sensor type: %d", sensor_type);
	ret = algorithm_initialization_by_sensor(g_decision_data, DECISION_DATA_LEN, sensor_type, reset_flag);
	if (ret != FP_OK)
	{
		Log("algorithm_initialization_by_sensor, algorithm_initialization fail ,ret = %d", ret);
		return FP_LIB_ERROR_GENERAL;
	}
exit:

	Log("%s, ret = %d", __func__, ret);

	return ret;
}

int doTestDestory()
{
	unsigned char out_data[DECISION_DATA_LEN];

	if (pSdk->pFeat != NULL)
	{
		Log("egis_fp_deInitAlg freeing pFeat");
		free(pSdk->pFeat);
		pSdk->pFeat = NULL;
	}
	if (pSdk->enroll_ctx != NULL)
	{
		Log("calling enroll_uninit");
		//enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}

	algorithm_uninitialization((unsigned char*)out_data, DECISION_DATA_LEN);
	{
		int i;
		for (i = 0; i < 5; i++)
			Log("egis_alg_deinit %d 0x%x", i, out_data[i]);
	}

	if (g_decision_data != NULL)
		memcpy(g_decision_data, out_data, DECISION_DATA_LEN);	
	/*
	 *	To keep decision data in TZApp, Since Huawei doesn't provide api to storage sensor/algo data.
	 *	Don't free g_decision_data here. 
	 */
	
	free(g_decision_data);
	
	//free(pSdk);
	return 0;
}

void doTestGetAlgoVersion()
{
	ALGO_API_INFO algo_api_version;
	char version[1024] = {0};
	algorithm_do_other(FP_ALGOAPI_GET_VERSION, NULL, (BYTE*)&algo_api_version);
	sprintf(version, "%s, %s", algo_api_version.algo_api_version, algo_api_version.matchlib_version);
	Log("algorithm version : %s", version);
	//printf("algorithm version : %s\n", version);
}


int traverse_dir_namelist(const char * path, struct dirent ***list, bool bfree)
{
	struct dirent **namelist;
	int itotalfile = 0;
	int iself = 0;
	itotalfile = scandir(path, &namelist, NULL, alphasort);
	if (itotalfile < 0) {
		Log("ERROR: count not open directory %s", path);
	}
	for (int i = 0; i < itotalfile; ++i) {
		if (namelist[i]->d_name[0] == '.') {
			//++iself;
			continue;
		}		
		//Log("%d: %s", i, namelist[i]->d_name);
		//printf("%d: %s\n", i, namelist[i]->d_name);
		if (bfree) {
			free(namelist[i]);
		}	
	}
	
	if (bfree) {
		free(namelist);
	}
	
	if (list != NULL) {
		*list = namelist;
	}
	
	Log("total files: %d", itotalfile - iself);
	return itotalfile/*  - iself */;
}

int load_db(char * path) 
{
	Log("Enter %s", __func__);
	int error_code = 0;
	struct dirent **namelist;
	int itotal = traverse_dir_namelist(path, &namelist, false);
	if (itotal < 0) return -1;
	g_db.ppmgr = (struct person **)malloc(sizeof(struct person *) * (itotal - 2));
	if (g_db.ppmgr == NULL) {
		free_namelist(namelist, itotal);
		return -1;
	}
	int i_person = 0;
	memset(g_db.ppmgr, 0, sizeof(struct person *) * (itotal - 2));
	//char append_path[MAX_LEN] = {'/'};
	for (int i = 0; i < itotal; ++i) {
		if (namelist[i]->d_name[0] == '.') {
			continue;
		}     
		g_db.ppmgr[i_person] = (struct person *)malloc(sizeof(struct person));		
		if (g_db.ppmgr[i_person] == NULL) {
			Log("fail(0)");
			//free_db();
			//return -1;
			error_code = -1;
			break;
		}
		memset(g_db.ppmgr[i_person], 0, sizeof(struct person));
		sprintf(g_db.ppmgr[i_person]->path, "%s/%s", path, namelist[i]->d_name);
        
		Log("ppmgr:%s", g_db.ppmgr[i_person]->path);
        //printf("ppmgr:%s\n", g_db.ppmgr[i_person]->path);
        g_db.person_count = i_person + 1;
		
        int ret = load_fingers(g_db.ppmgr[i_person]);
		if (ret == -1) {
			error_code = -1;
			break;
		}
		i_person++;		
	}
	
	free_namelist(namelist, itotal);
	
	
	return error_code;
}

void free_namelist(struct dirent **list, int itotalfile) 
{
	Log("Enter %s", __func__);
	for (int i = 0; i < itotalfile; ++i) {
		free(list[i]);
	}
	free(list);
}

void free_db()
{
	Log("Enter %s", __func__);
	for (int i = 0; i < g_db.person_count; ++i) {
		for (int ii = 0; ii < (g_db.ppmgr[i]->finger_count); ++ii) {
			//for (int iii = 0; iii < 2; ++iii) {
				if (g_db.ppmgr[i]->pp_f_mgr[ii]->e_v_status & IMG_TYPE_STATUS(ENROLL)) {
					//Log("free(ENROLL)");
					free_image_path(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[ENROLL]);
					free(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[ENROLL]);
				}				    
				if (g_db.ppmgr[i]->pp_f_mgr[ii]->e_v_status & IMG_TYPE_STATUS(VERIFY)) {
					//Log("free(VERIFY)");
					free_image_path(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[VERIFY]);
					free(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[VERIFY]);
				}					
			//}
			free(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr);
			free(g_db.ppmgr[i]->pp_f_mgr[ii]);
		}
		free(g_db.ppmgr[i]->pp_f_mgr);
		free(g_db.ppmgr[i]);
	}
	free(g_db.ppmgr);
	g_db.ppmgr = NULL;
}

int load_fingers(struct person * pp)
{
	Log("Enter: %s", __func__);
	int error_code = 0;
	struct dirent ** namelist;
	int itotal = traverse_dir_namelist(pp->path, &namelist, false);
	if (itotal < 0) return -1;
	pp->pp_f_mgr = (struct finger **)malloc(sizeof(struct finger *) * (itotal - 2));
	if (pp->pp_f_mgr == NULL) {
		free_namelist(namelist, itotal);
		return -1;
	}
	memset(pp->pp_f_mgr, 0, sizeof(struct finger *) * (itotal - 2));
	int i_finger = 0;
	for (int i = 0; i < itotal; ++i) {
		if (namelist[i]->d_name[0] == '.')
			continue;
		pp->pp_f_mgr[i_finger] = (struct finger *)malloc(sizeof(struct finger));
		if (pp->pp_f_mgr[i_finger] == NULL) {
			Log("fail(1)");
			//free_db();
			//return -1;
			error_code = -1;
			break;
		}
		pp->finger_count = i_finger + 1;
		memset(pp->pp_f_mgr[i_finger], 0, sizeof(struct finger));
		sprintf(pp->pp_f_mgr[i_finger]->path, "%s/%s", pp->path, namelist[i]->d_name);
        //Log("pp_f_mgr: %s", pp->pp_f_mgr[i_finger]->path);
		
		//finger belong its person
		pp->pp_f_mgr[i_finger]->of_p = pp;
		
        int ret = load_img_type(pp->pp_f_mgr[i_finger]);
		if (ret == -1) {
			error_code = -1;
			break;
		}
		++i_finger;
		
	}
	
    free_namelist(namelist, itotal);
	return error_code;
}

int load_img_type(struct finger * pf) {
	Log("Enter: %s", __func__);
	int error_code = 0;
	struct dirent ** namelist;
	int itotal = traverse_dir_namelist(pf->path, &namelist, false);
	if (itotal < 0) return -1;
	pf->pp_i_mgr = (struct img_attr **)malloc(sizeof(struct img_attr *) * (/* itotal -  */2));
	if (pf->pp_i_mgr == NULL) {
		free_namelist(namelist, itotal);
		return -1;
	}
	memset(pf->pp_i_mgr, 0, sizeof(struct img_attr *) * (itotal - 2));
	int e_v_status = 0;
	struct img_attr * p = NULL;
	for (int i = 0; i < itotal; ++i) {
		if (namelist[i]->d_name[0] == '.')
			continue;
		p = (struct img_attr *)malloc(sizeof(struct img_attr));
		if (p == NULL) {
			Log("fail(1)");
			//free_db();
			//return -1;
			error_code = -1;
			break;
		}
		memset(p, 0, sizeof(struct img_attr));
		
		if (strcmp(namelist[i]->d_name, "enroll") == 0) {
			if ((itotal - 2) > (ENROLL)) { // 
			    pf->pp_i_mgr[ENROLL] = p;
				sprintf(pf->pp_i_mgr[ENROLL]->path, "%s/%s/st", pf->path, namelist[i]->d_name);
				pf->e_v_status |= IMG_TYPE_STATUS(ENROLL);
				e_v_status = ENROLL;
			}
		} else if (strcmp(namelist[i]->d_name, "verify") == 0) {
			if ((itotal - 2) > (VERIFY)) { // 
			    pf->pp_i_mgr[VERIFY] = p;
				sprintf(pf->pp_i_mgr[VERIFY]->path, "%s/%s/st", pf->path, namelist[i]->d_name);
				pf->e_v_status |= IMG_TYPE_STATUS(VERIFY);
				e_v_status = VERIFY;
			}
		}
		pf->pp_i_mgr[e_v_status]->type = e_v_status;		
        Log("pp_f_mgr: %s", pf->pp_i_mgr[e_v_status]->path);
        load_image_path(pf->pp_i_mgr[e_v_status]);		
	}
	
    free_namelist(namelist, itotal);
	return error_code;
}

int pre_enroll() {
	
	int ret = 0;
	void *enroll_context = NULL;
	if (pSdk->pFeat != NULL) {
		free(pSdk->pFeat);
		pSdk->pFeat = NULL;
	}
	
	if (pSdk->enroll_ctx != NULL) {
		enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}
	
	if (pSdk->identify_session == TRUE) {
		verify_uninit();
	}
	
	do {
		if (pSdk->enroll_ctx == NULL) {
		    pSdk->pFeat = (unsigned char*)malloc(MAX_FEATURE_IDENTIFY);
		    if (pSdk->pFeat == NULL) {
			    Log("%s, %d, feat malloc fail(0)", __func__, __LINE__);
				ret = -1;
				break;
		    }
			
			enroll_context = enroll_init();
			if (enroll_context == NULL) {
				Log("%s, %d, enroll context fail(1)", __func__, __LINE__);
				free(pSdk->pFeat);
				pSdk->pFeat = NULL;
				ret = -1;
				break;
			}
			
			Log("enroll context  init, begin");
			set_enroll_context(enroll_context, ENROLL_CTX_MAX_ENROLL_COUNT, 12);
		    set_enroll_context(enroll_context, ENROLL_CTX_REDUNDANT_CHECK_START, 2);
		    set_enroll_context(enroll_context, ENROLL_CTX_REDUNDANT_CONTINUOUS_BOUND, 1);
			set_enroll_context(enroll_context, ENROLL_CTX_REDUNDANT_IMAGE_POLICY, FP_REDUNDANT_POLICY_ACCPET_IMAGE);
			
			pSdk->enroll_ctx = enroll_context;
		    pSdk->max_enroll_cnt = 0;
		    pSdk->enroll_result = 0;
		    pSdk->percentage = 0;
			Log("enroll context  init, end");
			
			ret = 0;
			break;
	    } else {
			Log("enroll context is not NULL");
		}
	} while (0);
	
	return ret;
}

int load_image_path(struct img_attr * pi) {
	Log("Enter: %s", __func__);
	int error_code = 0;
	struct dirent ** namelist;
	int itotal = traverse_dir_namelist(pi->path, &namelist, false);
	if (itotal < 0) return -1;
	pi->pp_e_v_mgr = (struct img_pic_path **)malloc(sizeof(struct img_pic_path *) * (itotal -  2));
	if (pi->pp_e_v_mgr == NULL) {
		free_namelist(namelist, itotal);
		return -1;
	}
	memset(pi->pp_e_v_mgr, 0, sizeof(struct img_pic_path *) * (itotal - 2));
	int e_v_c = 0;
	struct img_pic_path * p = NULL;
	for (int i = 0; i < itotal; ++i) {
		if (namelist[i]->d_name[0] == '.')
			continue;
		p = (struct img_pic_path *)malloc(sizeof(struct img_pic_path));
		if (p == NULL) {
			Log("fail(1)");
			error_code = -1;
			break;
		}
		memset(p, 0, sizeof(struct img_pic_path));
		pi->pp_e_v_mgr[e_v_c] = p;
		sprintf(pi->pp_e_v_mgr[e_v_c]->path, "%s/%s", pi->path, namelist[i]->d_name);
        //Log("pp_e_v_mgr: %s", pi->pp_e_v_mgr[e_v_c]->path);
		pi->img_count = ++e_v_c;
				
	}	
	free_namelist(namelist, itotal);
	return error_code;
}

void free_image_path(struct img_attr * pi) {
	//Log("Enter: %s", __func__);
	for (int i = 0; i < pi->img_count; ++i) {
		//Log("free xxx %s", pi->pp_e_v_mgr[i]->path);
		free(pi->pp_e_v_mgr[i]);
	}
	free(pi->pp_e_v_mgr);
}

int load_image(char *path, char *img, int *len) {
	int fd = 0;
	int ret = 0;
	int size = 0;
	
	//image size
	struct stat st;
	if (stat(path, &st) == 0) {
		size = st.st_size;
	}
	
	fd = open(path, O_RDONLY);
	if (fd <= 0) {	
		Log("%s, load image fail(2)", path);
		return -1;
	}
	memset(img, 0, *len);
	ret = read(fd, img, size);
	Log("%s, image %x %x %x %x %x", __func__, img[0], img[1], img[2], img[3], img[4]);
	*len = size;	
	close(fd);
	return ret; 
}

int identify_image_enroll(char * img, struct finger * f) 
{
	// 1. pass enrolled template to algorithm to check the current image (redundant, duplicate) or not 
	VERIFY_INIT set_enroll_db_data = {0};
	int has_enroll_templ = (f->of_p->p_tpl_cnt > 5) ? 5 : f->of_p->p_tpl_cnt;
	int retval = 0;
	
	if (f->of_p->p_tpl_cnt == 0 || f->of_p->pp_tmpl_mgr == NULL) {
		set_enroll_db_data.pEnroll_temp_array = NULL;
		set_enroll_db_data.enroll_temp_size_array = NULL;
		set_enroll_db_data.enroll_temp_number = 0;
	} else {
		Log("%s, %d", __func__, f->of_p->p_tpl_cnt);
		set_enroll_db_data.pEnroll_temp_array = (unsigned char **)malloc(has_enroll_templ * sizeof(unsigned char *));
		if (set_enroll_db_data.pEnroll_temp_array == NULL) {
			Log("Enroll temp array malloc fail(4)");
			return -1;
		}
		set_enroll_db_data.enroll_temp_size_array = (int *)malloc(has_enroll_templ * sizeof(int));
		if (set_enroll_db_data.enroll_temp_size_array == NULL) {
			free(set_enroll_db_data.pEnroll_temp_array);
			Log("Enroll temp size array malloc fail(4)");
			return -1;
		}
		
		Log("identify_image_enrollxxxx, %p, %p", set_enroll_db_data.pEnroll_temp_array, set_enroll_db_data.enroll_temp_size_array);
		set_enroll_db_data.enroll_temp_number = has_enroll_templ;
		Log("enroll_setDB: begin to set templ number= %d", set_enroll_db_data.enroll_temp_number);
		{
			for (int i = 0; i < has_enroll_templ; ++i) {
				Log("[DBG]pp_tpl_mgr[%d] = %p, size = %d", i, f->of_p->pp_tmpl_mgr[i]->tpl, f->of_p->pp_tmpl_mgr[i]->templ_size);
			}
			
			for (int i = 0; i < has_enroll_templ; ++i) {
				set_enroll_db_data.pEnroll_temp_array[i] = f->of_p->pp_tmpl_mgr[i]->tpl;
				set_enroll_db_data.enroll_temp_size_array[i] = f->of_p->pp_tmpl_mgr[i]->templ_size;
				Log("identify_image_enrollxxxxtempl (%d), , %p, %d", i, set_enroll_db_data.pEnroll_temp_array[i], set_enroll_db_data.enroll_temp_size_array[i]);
			}
		}
		
		Log("enroll_setDB: templ number= %d", set_enroll_db_data.enroll_temp_number);
		
		retval = enroll_setDB(pSdk->enroll_ctx, &set_enroll_db_data);
		if (retval != 0) {
			free(set_enroll_db_data.pEnroll_temp_array);
			free(set_enroll_db_data.enroll_temp_size_array);
			set_enroll_db_data.pEnroll_temp_array = NULL;
			set_enroll_db_data.enroll_temp_size_array = NULL;
			Log("%s, retval = %d", __func__, retval);
			return retval;
		}
	}
	
	do {
		retval = do_enroll(pSdk, img);
		if (retval != 0) {
			Log("%s, do_enroll fail ret = %d", __func__, retval);
			break;
		}

		switch (pSdk->enroll_result) {
			case FP_DUPLICATE:
			    Log("do_enroll, DUPLICATE"); // ENROLL_HELP_ALREADY_EXIST
			    break;
			case FP_MERGE_ENROLL_REDUNDANT_INPUT:
			    Log("do_enroll, FP_MERGE_ENROLL_REDUNDANT_INPUT"); // ENROLL_HELP_SAME_AREA
			    break;
			case FP_MERGE_ENROLL_FEATURE_LOW:
			    Log("do_enroll, FP_MERGE_ENROLL_FEATURE_LOW"); // ENROLL_FAIL_LOW_COVERAGE
			    break;
			case FP_MERGE_ENROLL_BAD_IMAGE:
			    Log("do_enroll, FP_MERGE_ENROLL_BAD_IMAGE"); //ENROLL_FAIL_LOW_QUALITY
			break;
            default: {
				if (pSdk->enroll_result <= 0) {
					Log("do_enroll, fail result %d", pSdk->enroll_result); // ENROLL_FAIL_NONE
				} else {
					Log("do_enroll, SUCCESS");
				}
			}
			break;
		}
		
	} while (0);
	
	free(set_enroll_db_data.pEnroll_temp_array);
	free(set_enroll_db_data.enroll_temp_size_array);
	set_enroll_db_data.pEnroll_temp_array = NULL;
	set_enroll_db_data.enroll_temp_size_array = NULL;
	
	return retval;
}

int finish_enroll(struct templ * template, IN char* tpl, IN int len) 
{
	int retval = 0;
	Log("Enter: %s", __func__);
	if ((template == NULL || tpl == NULL) && pSdk->enroll_result == FP_MERGE_ENROLL_FINISH) {
		Log("Enter: %s, (template %p), (tpl %p), (enroll_result %d) (2)", __func__, template, tpl, pSdk->enroll_result);
		return -1;
	}
	Log("enroll result = %d", pSdk->enroll_result);
	
	switch (pSdk->enroll_result) {
		case FP_MERGE_ENROLL_FINISH:		    
		    //Something else TODO
		    template->templ_size = MAX_ENROLL_TEMPLATE_LEN;
		    template->tpl = (unsigned char *)malloc(template->templ_size * sizeof(unsigned char));
			if (NULL == template->tpl) {
				Log("ERROR MEMORY");
				retval = -1;
				return retval;
			}
			memset(template->tpl, 0, MAX_ENROLL_TEMPLATE_LEN);
			template->templ_size = len;
			memcpy(template->tpl, tpl, len);
			
			Log("%s, template->tpl = %p, xxxxx tpl = %p", __func__, template->tpl, tpl);
			//free algo tpl
			//free(tpl);
			
		break;
		case FP_DUPLICATE:
		    Log("enroll_result: FP_DUPLICATE");
		break;
		case FP_MERGE_ENROLL_FAIL:
		case FP_MERGE_ENROLL_OUT_OF_MEMORY:
		case FP_MERGE_ENROLL_UNKNOWN_FAIL:
		case FP_MERGE_ENROLL_IRREGULAR_CONTEXT:
		default:
		    Log("enroll failed: Context need to be re-initialize: %d", pSdk->enroll_result);
			break;
	}
	
	if (pSdk->pFeat != NULL) {
		free(pSdk->pFeat);
		pSdk->pFeat = NULL;
	}
	if (pSdk->enroll_ctx != NULL) {
		enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}
	
	pSdk->enrolled_cnt = 0;
	pSdk->max_enroll_cnt = 0;
	
	Log("END: %s", __func__);
	return retval;
}

static int g_duplicate_enroll = 0;
int do_enroll(struct alg_info* pSdk, char * img) 
{
	int width = WIDTH;
	int height = HEIGHT;
	
	pSdk->enroll_result = enroll(img, width, height, pSdk->enroll_ctx, &(pSdk->percentage));
	get_enroll_context(pSdk->enroll_ctx, ENROLL_CTX_MAX_ENROLL_COUNT, (INT*) &(pSdk->max_enroll_cnt));
	get_enroll_context(pSdk->enroll_ctx, ENROLL_CTX_ENROLLED_COUNT, (INT*) &(pSdk->enrolled_cnt));
	Log("enrolled_cnt = %d (max %d)", pSdk->enrolled_cnt, pSdk->max_enroll_cnt);
	if (pSdk->enroll_result == FP_DUPLICATE) {
		g_duplicate_enroll++;
	}
	
	pSdk->enrolled_cnt -= g_duplicate_enroll;
	if (pSdk->max_enroll_cnt > 0) {
		pSdk->percentage = pSdk->enrolled_cnt * 100 / pSdk->max_enroll_cnt;
		if (pSdk->percentage > 100) {
			pSdk->percentage = 100;
		}
	}
	
	Log("do_enroll percent(1) = %d, enroll_result = %d enrolled_cnt = %d max_enroll_cnt = %d",
		pSdk->percentage, pSdk->enroll_result, pSdk->enrolled_cnt, pSdk->max_enroll_cnt);
	
	return 0;
}

int update_enroll_templ(OUT char ** tpl, OUT int *len)  
{
	Log("Enter: %s", __func__);
	int retval = -1;
	if (pSdk->percentage == 100) {
		pSdk->enroll_result = FP_MERGE_ENROLL_FINISH;
		Log("update_enroll_templ,!!! Enroll_Finish !!!");
		
		*tpl = get_enroll_template(pSdk->enroll_ctx, len);
		retval = 0;
	}
	
	Log("%s tpl=%p,  len = %d, retval = %d", __func__, *tpl, *len, retval);
	
	Log("End: %s", __func__);
	return retval;
}

void free_templ_mgr()
{
	if (g_db.pp_tmpl_mgr != NULL) {
		for (int i = 0; i < g_db.templ_count; ++i) {
			free(g_db.pp_tmpl_mgr[i]->tpl);
			free(g_db.pp_tmpl_mgr[i]);
		}
		free(g_db.pp_tmpl_mgr);
	}
	
	g_db.pp_tmpl_mgr = NULL;
	g_db.templ_count = 0;
}

int enroll_one_finger(struct finger * f)
{
	int retval = 0; 
	if (f == NULL) {
		return -1;
	}
	
	pre_enroll();
	
	if (f->of_p->pp_tmpl_mgr == NULL) {
		Log("person template");
		f->of_p->pp_tmpl_mgr = (struct templ **)malloc(sizeof(struct templ *) * f->of_p->finger_count);
	}
	
	int len = WIDTH * HEIGHT;
	char img[WIDTH * HEIGHT] = {0};
	for (int i = 0; i < f->pp_i_mgr[ENROLL]->img_count; ++i) {//traverse enroll folder
	    // 1. capture image
		load_image(f->pp_i_mgr[ENROLL]->pp_e_v_mgr[i]->path, img, &len);
		Log("%s, load img size %d", __func__, len);
		
		// 2. identify image enroll
		identify_image_enroll(img, f);
		if (pSdk->enroll_result == FP_DUPLICATE/* FP_LIB_ENROLL_HELP_ALREADY_EXIST */) {
			Log("%s, FP_LIB_ENROLL_HELP_ALREADY_EXIST", __func__);
			break;
		} else if (pSdk->enroll_result == FP_MERGE_ENROLL_REDUNDANT_INPUT/* FP_LIB_ENROLL_HELP_SAME_AREA */) {
			Log("%s, FP_LIB_ENROLL_HELP_SAME_AREA", __func__);
			continue;
		} else if (pSdk->enroll_result == FP_MERGE_ENROLL_IAMGE_OK){
			Log("%s, Enroll result: FP_MERGE_ENROLL_IAMGE_OK", __func__);
			continue;
			//break;
		} else if (pSdk->enroll_result == FP_MERGE_ENROLL_FINISH) {
			Log("%s, Enroll result: FP_MERGE_ENROLL_FINISH", __func__);
		} else if (pSdk->enroll_result == FP_MERGE_ENROLL_BAD_IMAGE) {
			Log("%s, Enroll result: FP_MERGE_ENROLL_BAD_IMAGE", __func__);
			continue;
		} else if (pSdk->enroll_result <= 0) {
			Log("%s, enroll result ERROR", __func__);
			break;
		}
		
		if (pSdk->percentage == 100) {
			Log("Enroll Finish, result = %d", pSdk->enroll_result);
			// 3. update templ
		    char * tpl_c = NULL;
		    int len = 0;
		    update_enroll_templ(&tpl_c, &len);
			pSdk->enroll_result = FP_MERGE_ENROLL_FINISH;
		    // 4. finish enroll
			struct templ* tpl = (struct templ *)malloc(sizeof(struct templ));
			if (tpl == NULL) {
				Log("MEMORY ALLOC FAIL");
				retval = -1;
				break;
			}
			memset(tpl, 0, sizeof(struct templ));
		    finish_enroll(tpl, tpl_c, len);
			memcpy(tpl->templ_path, f->path, strlen(f->path));
			tpl->fid = g_db.templ_count;
			f->fid = g_db.templ_count;
			g_db.pp_tmpl_mgr[g_db.templ_count++] = tpl;
			f->of_p->pp_tmpl_mgr[f->of_p->p_tpl_cnt++] = tpl;
			for (int i = 0; i < f->of_p->p_tpl_cnt; ++i) {
				Log("[DBG]person tpl[%d] = %p, size = %d", i, f->of_p->pp_tmpl_mgr[i]->tpl, f->of_p->pp_tmpl_mgr[i]->templ_size);
			}
			Log("%s, person templ_cnt(%d)", __func__, f->of_p->p_tpl_cnt);
		}
			
	}
	
	Log("%s, person total templ_cnt(%d)", __func__, f->of_p->p_tpl_cnt);
	return retval;
	
}

int doTestEnroll() 
{
	//Init template array
    free_templ_mgr();
    g_db.total_finger_cnt = 0;
	for (int i = 0; i < g_db.person_count; ++i) {
		g_db.total_finger_cnt += g_db.ppmgr[i]->finger_count;
	}
	Log("Enter %s, total finger: %d", __func__, g_db.total_finger_cnt);
	
	g_db.pp_tmpl_mgr = (struct templ **)malloc(sizeof(struct templ *) * g_db.total_finger_cnt);
	if (g_db.pp_tmpl_mgr == NULL) {
		Log("malloc fail(0)");
		return -1;
	}
	memset(g_db.pp_tmpl_mgr, 0 , sizeof(struct templ *) * g_db.total_finger_cnt);
	for (int i = 0; i < g_db.person_count; ++i) {
		for (int ii = 0; ii < g_db.ppmgr[i]->finger_count; ++ii) {
			Log("(person, finger) = (%d, %d)", i, ii);
			enroll_one_finger(g_db.ppmgr[i]->pp_f_mgr[ii]);
		}
	}
	
	return 0;
}

void do_verify_uninit(void) 
{
	if (pSdk->identify_session) {
		verify_uninit();
	}
	pSdk->identify_session = FALSE;
	return;
}

int pre_verify(struct templ * t)
{
	int retval = 0;
	if (t == NULL) {
		Log("%s, null pointer", __func__);
		return -1;
	}
	
	if (pSdk->pFeat == NULL) {
		pSdk->pFeat = (unsigned char *)malloc(MAX_FEATURE_IDENTIFY);
		Log("%s, MAX_FEATURE_IDENTIFY(expect=%d)=%d", __func__, 16 * 1024, MAX_FEATURE_IDENTIFY);
		if (NULL == pSdk->pFeat) {
			Log("%s, Feat allocate fail(5)", __func__);
			return -1;
		}
		memset(pSdk->pFeat, 0, MAX_FEATURE_IDENTIFY);
	}
	
	if (pSdk->enroll_ctx != NULL) {
		Log("%s, calling enroll_uninit!", __func__);
		enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}
	
	do_verify_uninit();
	
	VERIFY_INIT verify_init_data = {0};
	do {
		Log("%s, start!", __func__);
	    
	    int template_number = 1;//I only want to verify one template.
	    verify_init_data.pEnroll_temp_array = (unsigned char **)malloc(template_number * sizeof(unsigned char *));
	    if (NULL == verify_init_data.pEnroll_temp_array) {
			
			retval = -1;
			break;
		}
		
		verify_init_data.enroll_temp_size_array = (int *)malloc(template_number * sizeof(int));
		if (NULL == verify_init_data.enroll_temp_size_array) {
			retval = -1;
			break;
		}
		
		for (int i = 0; i < template_number; ++i) {
			verify_init_data.pEnroll_temp_array[i] = t->tpl;
			verify_init_data.enroll_temp_size_array[i] = t->templ_size;
			Log("%s, verify[%d] tpl=%p, size=%d", __func__, i, verify_init_data.pEnroll_temp_array[i], verify_init_data.enroll_temp_size_array[i]);
		}
		
		verify_init_data.enroll_temp_number = template_number;
		
		retval = verify_init(&verify_init_data);
		
		pSdk->identify_session = TRUE;
		
	} while (0);
	
	//if (retval == -1) {
		if (verify_init_data.pEnroll_temp_array != NULL) {
			free(verify_init_data.pEnroll_temp_array);
		}
		if (verify_init_data.enroll_temp_size_array != NULL) {
			free(verify_init_data.enroll_temp_size_array);
		}
	//}
	
	Log("%s, ret=%d", __func__, retval);
	return retval;
}

void finish_verify()
{
	if (pSdk->pFeat != NULL) {
        free(pSdk->pFeat);
		pSdk->pFeat = NULL;
	}
	
	if (pSdk->enroll_ctx != NULL) {
		Log("%s, calling enroll_uninit!", __func__);
		enroll_uninit(pSdk->enroll_ctx);
		pSdk->enroll_ctx = NULL;
	}
	
	do_verify_uninit();
}

int identify_image_verify(char * img, struct templ * t, int fid) 
{
	int retval = 0;
	int exret = 0;
	int extract_quality = 0;
	
	if (pSdk->pFeat == NULL) {
		pSdk->pFeat = (unsigned char *)malloc(MAX_FEATURE_IDENTIFY);
		Log("%s, MAX_FEATURE_IDENTIFY(expect=%d)=%d", __func__, 16 * 1024, MAX_FEATURE_IDENTIFY);
		if (NULL == pSdk->pFeat) {
			Log("%s, Feat allocate fail(5)", __func__);
			return -1;
		}
		memset(pSdk->pFeat, 0, MAX_FEATURE_IDENTIFY);
	}
	
	if (img == NULL) {
		Log("%s, get image fail.", __func__);
		return -1;
	}
	
	exret = extract_feature(img, WIDTH, HEIGHT, pSdk->pFeat, &(pSdk->m_ftsize), &extract_quality);
	Log("%s, extract_feature size = %d , extract_quality = %d , exret = %d(OK=0)", __func__, pSdk->m_ftsize, extract_quality, exret);
	
	//do algo verify
	VERIFY_INFO verify_info = {0};
	verify_info.pFeat = pSdk->pFeat;
	verify_info.feat_size = pSdk->m_ftsize;
	verify_info.extract_ret = exret;
	verify_info.enroll_temp_size = 0;
	verify_info.pEnroll_temp = (unsigned char *)malloc(MAX_ENROLL_TEMPLATE_LEN);
	verify_info.matchScore = 0;
	verify_info.matchindex = -1;
	verify_info.isLearningUpdate = FALSE;
	if (verify_info.pEnroll_temp == NULL) {
		Log("%s, Enroll_temp allocate fail(6)", __func__);
		return -1;
	}
	
	retval = verify(&verify_info);
	Log("%s, VERIFY result(%d)", __func__, retval);
	if (retval >= 0) {
		retval = 0;
	} else if (retval == FP_MATCHFAIL) {
		retval = FP_MATCHFAIL;
	} else {
		retval = FP_MATCHFAIL;
	}
	
	pSdk->matched_bioidx = verify_info.matchindex;
	pSdk->isLearningUpdate = verify_info.isLearningUpdate;
	pSdk->matched_score = verify_info.matchScore;
	
	Log("STATE_VERIFY isLearningUpdate = %d, match index = %d, match score = %d", pSdk->isLearningUpdate, pSdk->matched_bioidx, pSdk->matched_score);
	
	if (pSdk->isLearningUpdate && (t->fid == fid)) { //only this finger's image match success, can update template.
		Log("%s[DBG] Learning Update before, fid=%d, t->tpl = %p, t->templ_size = %d", __func__, fid, t->tpl, t->templ_size);
		t->tpl = (unsigned char *)realloc(t->tpl, verify_info.enroll_temp_size);
		memcpy(t->tpl, verify_info.pEnroll_temp, verify_info.enroll_temp_size);
		t->templ_size = verify_info.enroll_temp_size;
		Log("%s[DBG] Learning Update after, fid=%d, t->tpl = %p, t->templ_size = %d", __func__, fid, t->tpl, t->templ_size);
	}
	
	free(verify_info.pEnroll_temp);
	return retval;
	
}

int doTestVerify(struct templ * t) 
{
	if (t == NULL) { //init db template, and verify all image to everyone template.
		doTestEnroll();
		for (int t = 0; t < g_db.templ_count; ++t) {
			doTestVerify(g_db.pp_tmpl_mgr[t]);
		}
		return 0;
	}
	
	int len = WIDTH * HEIGHT;
	char img[WIDTH * HEIGHT] = {0};
	int retval = 0;
	//traverse all image
	for (int i = 0; i < g_db.person_count; ++i) { //traverse all person
		for (int ii = 0; ii < g_db.ppmgr[i]->finger_count; ++ii) { // traverse all finger
			for (int iii = 0; iii < g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[VERIFY]->img_count; ++iii) { // traverse all image
				// 1.
				pre_verify(t);
				// 2. capture image
				load_image(g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[VERIFY]->pp_e_v_mgr[iii]->path, img, &len);
				Log("%s, load img size %d", __func__, len);
				Log("%s, image %x %x %x %x %x", __func__, img[0], img[1], img[2], img[3], img[4]);
				// 3. identify
				retval = identify_image_verify(img, t, g_db.ppmgr[i]->pp_f_mgr[ii]->fid);
				if (t->fid == g_db.ppmgr[i]->pp_f_mgr[ii]->fid) { //care frr
					g_far_frr.total_fr++;					
					if (retval != 0) {
						g_far_frr.fr++;
					}
					Log("[DBG][FR]FR = %d, total = %d", g_far_frr.fr, g_far_frr.total_fr);
				} else { // care far
					g_far_frr.total_fa++;
					if (retval == 0) {
						g_far_frr.fa++;
					}
					Log("[DBG][FA]FA = %d, total = %d", g_far_frr.fa, g_far_frr.total_fa);
				}
				Log("[DBG][fid]t = %d, image = %d", t->fid, g_db.ppmgr[i]->pp_f_mgr[ii]->fid);
				Log("[DBG][FRR|FAR]=%d, %s, %s", retval, t->templ_path, g_db.ppmgr[i]->pp_f_mgr[ii]->pp_i_mgr[VERIFY]->pp_e_v_mgr[iii]->path);				
			}
		}
	}
	finish_verify();
	return 0;
}

void doTestAlgo()
{
	Log("Enter: %s", __func__);
	doTestInit();
	doTestGetAlgoVersion();
	//traverse_dir_namelist("/sdcard/fingerprint/0001/0/enroll/st", NULL, true);
	load_db(ROOT_PATH);
	//doTestEnroll();
	doTestVerify(NULL);
	Log("---------------------------");
	Log("FR = %d, FRR = %f%%, total = %d", g_far_frr.fr, (g_far_frr.fr / (float) g_far_frr.total_fr) * 100, g_far_frr.total_fr);
	Log("FA = %d, FAR = %f%%, total = %d", g_far_frr.fa, (g_far_frr.fa / (float) g_far_frr.total_fa) * 100, g_far_frr.total_fa);
	free_db();
	doTestDestory();
}


























