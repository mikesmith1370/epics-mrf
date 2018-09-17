TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += mrfEpicsUdpIp

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install mrfUdpIp.dbd into <top>/dbd
DBD += mrfUdpIp.dbd

# specify all source files to be compiled and added to the library
mrfEpicsUdpIp_SRCS += mrfRegistrarUdpIp.cpp

mrfEpicsUdpIp_LIBS += $(EPICS_BASE_IOC_LIBS)
mrfEpicsUdpIp_LIBS += mrfCommon
mrfEpicsUdpIp_LIBS += mrfEpics
mrfEpicsUdpIp_LIBS += mrfUdpIp

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

