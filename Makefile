# make all demoes at one time

#CROSS_COMPILE := arm-linux-gnueabihf-
CROSS_COMPILE := arm-myir-linux-gnueabihf-
PRU_CGT :=/home/sunny/toolchain_linaro/ti-cgt-pru_2.1.3

SUBDIRS=framebuffer keypad rtc eeprom led can camera tty spi gpio watchdog beeper pru_led

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

clean:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done

.PHONY: all clean $(SUBDIRS)

