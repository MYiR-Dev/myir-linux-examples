# make all demoes at one time

CROSS_COMPILE := arm-linux-gnueabihf-

all: 
	make -C ./framebuffer/
	make -C ./keypad/
	make -C ./rtc/
	make -C ./eeprom/
	make -C ./led/
	make -C ./can/
	make -C ./camera/
	make -C ./rs232/
	make -C ./rs485/
	make -C ./spi/
	make -C ./gpio/
	make -C ./watchdog/
	make -C ./beeper/
	

.Phony: clean

clean:
	make clean -C ./framebuffer/
	make clean -C ./keypad/
	make clean -C ./rtc/
	make clean -C ./eeprom/
	make clean -C ./led/
	make clean -C ./can/
	make clean -C ./camera/
	make clean -C ./rs232/
	make clean -C ./rs485/
	make clean -C ./spi/
	make clean -C ./gpio/
	make clean -C ./watchdog/
	make clean -C ./beeper/


