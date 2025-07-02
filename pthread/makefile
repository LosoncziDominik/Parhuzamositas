CC = gcc
CFLAGS = -Wall -O2 -I"C:\msys64\mingw64\include" -fopenmp
LDFLAGS = -L"C:\msys64\mingw64\lib" -lmingw32 -lSDL2main -lSDL2 -lgomp -mconsole -lSDL2_ttf -O2 -lpthread

boid_sim.exe: main.o boid.o
	$(CC) -o boid_sim.exe main.o boid.o $(LDFLAGS)

main.o: main.c boid.h
	$(CC) $(CFLAGS) -c main.c -o main.o

boid.o: boid.c boid.h
	$(CC) $(CFLAGS) -c boid.c -o boid.o

clean:
	rm -f *.o boid_sim.exe
