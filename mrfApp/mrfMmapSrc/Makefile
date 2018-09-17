TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

# At the moment, we can only build this module on Linux because the MRF device
# driver is not available for other platforms and we are also relying on some
# platform specific behavior regarding memory-mapped I/O and signals.
LIBRARY_IOC_Linux += mrfMmap

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install mrfMmap.dbd into <top>/dbd
#DBD += mrfMmap.dbd

INC += MrfMmapMemoryAccess.h

# specify all source files to be compiled and added to the library
mrfMmap_SRCS += MrfMmapMemoryAccess.cpp

# mrfMmap_LIBS += $(EPICS_BASE_IOC_LIBS)
mrfMmap_LIBS += mrfCommon

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

