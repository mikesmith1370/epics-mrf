# Comment/uncomment to allow detection of unconfigured board
#DETECT_UNCONFIGURED_BOARD=y
# Comment/uncomment to hide/show debugging messages in system log
DEBUG=y

ifeq ($(DETECT_UNCONFIGURED_BOARD),y)
  EXTRA_CFLAGS += -DDETECT_UNCONFIGURED_BOARD
endif
ifeq ($(DEBUG),y)
  EXTRA_CFLAGS += -DDEBUG
endif

pci_mrfevr-objs := flash.o mrfcommon.o eeprom_9056.o eeprom.o \
                   jtag_9056.o jtag.o mrfevr.o
pci_mrfevg-objs := flash.o mrfcommon.o eeprom_9056.o eeprom.o \
                   jtag_9056.o jtag.o mrfevg.o

obj-m := pci_mrfevr.o pci_mrfevg.o

KVERSION := $(shell uname -r)

all:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
