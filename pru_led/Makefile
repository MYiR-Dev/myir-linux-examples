SUBDIRS=pru_led lib/src PRU_RPMsg_LED0_1 PRU_RPMsg_Echo_Interrupt0_0 PRU_Halt

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

clean:
	@for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done

.PHONY: all clean $(SUBDIRS)



