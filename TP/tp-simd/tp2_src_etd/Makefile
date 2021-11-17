SHELL=/bin/bash
CFLAGS=-O3 -Wall -march=native # native = on cible la machine sur laquelle on compile. Attention cependant, certaines vieilles machines n'auront peut-être pas toutes les extensions que vous souhaitez utiliser
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

mjpeg-mmx : conv-mmx.o $(OBJS)
	gcc $(CFLAGS) -Bstatic -o $@ $^ -lSDL

mjpeg-conv-unrolled4-float-a-trou : conv-unrolled4-float-a-trou.o $(OBJS)
	gcc  $(CFLAGS) -Bstatic -o $@ $^ -lSDL

mjpeg-conv-sse-a-trou : conv-sse-a-trou.c $(OBJS)
	gcc  $(CFLAGS) -Bstatic -o $@ $^ -lSDL

realclean : clean
	rm -f mjpeg-float mjpeg-int mjpeg-mmx mjpeg-unrolled4-float-a-trou mjpeg-conv-sse-a-trou
clean :
	rm -f conv-float.o conv-int.o conv-unrolled4-float-a-trou.o conv-sse-a-trou.o
