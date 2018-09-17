TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC_Linux += mrfEpicsMmap

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install mrfMmap.dbd into <top>/dbd
DBD += mrfMmap.dbd

# specify all source files to be compiled and added to the library
mrfEpicsMmap_SRCS += mrfRegistrarMmap.cpp

mrfEpicsMmap_LIBS += $(EPICS_BASE_IOC_LIBS)
mrfEpicsMmap_LIBS += mrfCommon
mrfEpicsMmap_LIBS += mrfMmap

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

