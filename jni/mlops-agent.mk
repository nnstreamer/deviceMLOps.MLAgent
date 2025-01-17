ifndef MLOPS_AGENT_ROOT
$(error MLOPS_AGENT_ROOT is not defined!)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := sqliteX
LOCAL_SRC_FILES := $(MLOPS_AGENT_ROOT)/libsqliteX/$(APP_ABI)/libsqliteX.so
LOCAL_EXPORT_C_INCLUDES := $(MLOPS_AGENT_ROOT)/libsqliteX/include
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
# mlops agent headers
MLOPS_AGENT_INCLUDE := $(MLOPS_AGENT_ROOT)/daemon/include

# mlops agent sources
MLOPS_AGENT_SRCS := $(MLOPS_AGENT_ROOT)/daemon/mlops-agent-android.c
LOCAL_SHARED_LIBRARIES := sqliteX
include $(BUILD_SHARED_LIBRARY)

