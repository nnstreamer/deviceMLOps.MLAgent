ifndef MLOPS_AGENT_ROOT
$(error MLOPS_AGENT_ROOT is not defined!)
endif

# mlops agent headers
MLOPS_AGENT_INCLUDE := $(MLOPS_AGENT_ROOT)/daemon/include

# mlops agent sources
MLOPS_AGENT_SRCS := \
    $(MLOPS_AGENT_ROOT)/daemon/mlops-agent-android.c \
    $(MLOPS_AGENT_ROOT)/daemon/mlops-agent-internal.c \
    $(MLOPS_AGENT_ROOT)/daemon/mlops-agent-node.c \
    $(MLOPS_AGENT_ROOT)/daemon/service-db.cc
