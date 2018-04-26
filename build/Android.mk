LOCAL_PATH:= $(call my-dir)
LOCAL_CODE_PATH := $(LOCAL_PATH)/../
LOCAL_BASE_CODE_PATH := $(LOCAL_CODE_PATH)/../../


LOCAL_MODULE := biosign
LOCAL_MODULE_FILENAME := libbiosign
LOCAL_C_INCLUDES := \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/inc/
LOCAL_SRC_FILES := \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/G3AlgoLib/NONTZ/libEgisG3Algorithm_arm64-v8a_standard.a	
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

EGIS_ALGORITHM_SRCS = \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/EgisImageAlgorithm.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/FPMatchLib.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/Memory.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFContext.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFEBnrz.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFEFltr.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFESklt.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFtCoD.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFtExt.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFFtMat.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFinger.c \
	$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/src/VFSys.c
	
	
LOCAL_SRC_FILES := $(LOCAL_CODE_PATH)/main.c \
                   $(LOCAL_CODE_PATH)/test_algo_api.c \
				   $(LOCAL_CODE_PATH)/log.c
				   
LOCAL_SRC_FILES += $(EGIS_ALGORITHM_SRCS) \
                   $(LOCAL_BASE_CODE_PATH)/native/libfpets/commonlib/general/src/Egis_log.c \
				   $(LOCAL_BASE_CODE_PATH)/native/libfpets/egiscipherlib/src/AES.c


	
				   
LOCAL_C_INCLUDES += $(LOCAL_CODE_PATH) \
                    $(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisLibModule/MatchingAlgorithm/inc/ \
					$(LOCAL_BASE_CODE_PATH)/middleware/fingerprint/fingerprint_module/include/ \
					$(LOCAL_BASE_CODE_PATH)/native/libFpEts/CommonLib/egis_inc/inc/ \
					$(LOCAL_BASE_CODE_PATH)/native/libFpEts/CommonLib/general/inc/ \
					$(LOCAL_BASE_CODE_PATH)/native/libFpEts/EgisCipherLib/inc/
					
LOCAL_CFLAGS += -D__ET538__ 
LOCAL_CFLAGS += -DHW_HAWAII_Y6
LOCAL_CFLAGS += -D__LINUX__
LOCAL_CFLAGS += -v
LOCAL_CFLAGS += -DTEEI -DTZ_MODE
LOCAL_CFLAGS += -DG3_MATCHER 

LOCAL_STATIC_LIBRARIES += libbiosign
					
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := Talgo
include $(BUILD_EXECUTABLE)