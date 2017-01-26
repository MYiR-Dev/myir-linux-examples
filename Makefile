# make all demoes at one time

export PRU_CGT=/home/sunny/toolchain_linaro/ti-cgt-pru_2.1.3

#PREFIX ?= $(shell pwd)/rootfs
PREFIX = /home/sunny/export/rootfs

#CROSS_COMPILE := arm-linux-gnueabihf-
CROSS_COMPILE := arm-myir-linux-gnueabihf-

SUBDIRS=framebuffer keypad rtc eeprom led can tty gpio pru_led

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

clean:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done
	
install:
	mkdir -p $(PREFIX)/lib/firmware
	mkdir -p $(PREFIX)/usr/bin
	cp pru_led/PRU_RPMsg_Echo_Interrupt0_0/gen/PRU_RPMsg_Echo_Interrupt0_0.out  $(PREFIX)/lib/firmware/am437x-pru0_0-fw
	cp pru_led/PRU_RPMsg_LED0_1/gen/PRU_RPMsg_LED0_1.out  $(PREFIX)/lib/firmware/am437x-pru0_1-fw
	@for d in $(SUBDIRS); do (cd $$d; cp $$d"_test" $(PREFIX)/usr/bin/ ); done


.PHONY: all clean $(SUBDIRS) install



