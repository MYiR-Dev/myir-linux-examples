# make all demoes at one time

all: 
	make -C ./can/
	make -C ./leds/

.Phony: clean

clean:
	make clean -C ./can/
	make clean -C ./leds/
