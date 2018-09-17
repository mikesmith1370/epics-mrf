TOP = ..
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Db*))

mrfEpicsMmapSrc_DEPEND_DIRS = mrfCommonSrc mrfEpicsSrc mrfMmapSrc
mrfEpicsSrc_DEPEND_DIRS = mrfCommonSrc
mrfEpicsUdpIpSrc_DEPEND_DIRS = mrfCommonSrc mrfEpicsSrc mrfUdpIpSrc
mrfMmapSrc_DEPEND_DIRS = mrfCommonSrc
mrfUdpIpSrc_DEPEND_DIRS = mrfCommonSrc

include $(TOP)/configure/RULES_DIRS
