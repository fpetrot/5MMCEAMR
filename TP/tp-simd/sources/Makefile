INC = -I.
CFLAGS = -g -O3 $(INC) -Wall -Werror -march=sandybridge
LDFLAGS = -lSDL2

CONV=conv-xxx

OBJ_FILES = main.o idct.o screen.o iqzz.o upsampler.o skip_segment.o huffman.o unpack_block.o $(CONV).o

all : mjpeg

run :
	 ./mjpeg ../tp2_src/ice_age_256x144_444.mjpeg

#Rules for beginners
mjpeg : $(OBJ_FILES)
	gcc -o mjpeg $(OBJ_FILES) -lSDL
main.o : main.c jpeg.h screen.h upsampler.h huffman.h skip_segment.h idct.h iqzz.h conv.h utils.h define_common.h unpack_block.h
	gcc $(CFLAGS) -std=c99 -c main.c
idct.o : idct.c idct.h
	gcc $(CFLAGS) -std=c99 -c idct.c 
screen.o : screen.c screen.h
	gcc $(CFLAGS) -std=c99 -c screen.c 
iqzz.o : iqzz.c iqzz.h
	gcc $(CFLAGS) -std=c99 -c iqzz.c
upsampler.o : upsampler.c upsampler.h
	gcc $(CFLAGS) -std=c99 -c upsampler.c
skip_segment.o : skip_segment.c skip_segment.h
	gcc $(CFLAGS) -std=c99 -c skip_segment.c
huffman.o : huffman.c huffman.h
	gcc $(CFLAGS) -std=c99 -c huffman.c
unpack_block.o : unpack_block.c unpack_block.h
	gcc $(CFLAGS) -std=c99 -c unpack_block.c

# Force la reconstruction indépendamment de la date
$(CONV).o : $(CONV).c conv.h FORCE
	gcc $(CFLAGS) -std=c99 -c $(CONV).c

.PHONY: FORCE
FORCE:

clean:
	rm -f mjpeg $(OBJ_FILES)
