NAME = liu_bouri # nom1_nom2
RPI ?= 23 # card_number

CROSS_COMPILE ?= bcm2708hardfp-

CFLAGS=-Wall -Wfatal-errors -O2 
LDFLAGS=-static -lpthread

help:
	@echo ""
	@echo "make [RPI=num] target"
	@echo ""
	@echo "      target :	help	this message"
	@echo " 		all	clean all and recompile"
	@echo " 		dep	compile if required"
	@echo " 		clean	clean all except sources"
	@echo " 		copy	compile then copy to card $(RPI) in directory $(NAME)"
	@echo ""

all: clean dep 

dep : blink01_pt.x

copy: *.x
	scp -P 622$(RPI) $^ pi@peri:$(NAME)

%.x: %.c
	$(CROSS_COMPILE)gcc -D_REENTRANT -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.x *~ *.a
