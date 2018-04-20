LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS += -Wall -std=c99 -I$(LOCAL_PATH)/../../lib -ffast-math -O2 -DNDEBUG
#LOCAL_LDFLAGS += -llog
LOCAL_MODULE    := ccr
LOCAL_SRC_FILES :=\
	ccr.c\
	../../lib/kanji.c\
	../../lib/dist_assign_strokes.c\
	../../lib/dist_stroke.c\
	../../lib/wmcbm.c\
	../../lib/feedback.c\
	../../lib/lookup_strokecnt.c
include $(BUILD_SHARED_LIBRARY)
