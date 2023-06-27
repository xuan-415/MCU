
filename = main
my_prog :
	sdcc $(filename).c
	packihx $(filename).ihx > $(filename).hex && make clean
clean :
	del $(filename).lk
	del $(filename).lst
	del $(filename).map
	del $(filename).mem
	del $(filename).rel
	del $(filename).asm
	del $(filename).rst
	del $(filename).sym