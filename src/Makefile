CC = gcc

# Important we need to turn optimization on for the cpu driver
CFLAGS = -march=native -Ofast -funroll-loops -fomit-frame-pointer 

OBJS	= main.o fb.o libC_assist.o cpu.o mem.o time.o mmu.o mbox.o


all: $(OBJS)
	$(CC) $(CFLAGS) -o 64xpi $(OBJS) -lpthread -I/usr/include/SDL2 -lSDL2


install:
	install -m 755 64xpi /usr/bin/64xpi

clean:
	rm -f $(OBJS) 64xpi
