SHELL=/bin/bash
CFLAGS=-O3 -Wall -march=core2 # pcserveur est un sandybridge,  vérifier sur les machines de la salle de TP
CONV=conv-int
OBJS=huffman.o idct.o iqzz.o main.o screen.o skip_segment.o unpack_block.o upsampler.o

all :
	@echo "Tapez make mjpeg-xxx pour construire le décodeur utilisant le convertisseur xxx"
	@echo "Possibilités :"
	@gawk -F : '/^mjpeg/{print "make", $$1}' Makefile

mjpeg-float : conv-float.o $(OBJS)
	gcc -Bstatic -o $@ $^ -lSDL

mjpeg-int : conv-int.o $(OBJS)
	gcc -Bstatic -o $@ $^ -lSDL

mjpeg-unrolled4 : conv-unrolled4.o $(OBJS)
	gcc -Bstatic -o $@ $^ -lSDL

mjpeg-mmx : conv-mmx.o $(OBJS)
	gcc -Bstatic -o $@ $^ -lSDL


realclean : clean
	rm -f mjpeg-float mjpeg-int mjpeg-mmx mjpeg-unrolled4
clean :
	rm -f conv-float.o conv-int.o conv-mmx.o conv-unrolled4.o
