default: chip8

chip8.o: chip8.c 
	gcc -c chip8.c -o chip8.o 

chip8: chip8.o
	gcc chip8.o -o chip8 -lSDL2

clean:
	rm -rf *.o chip8
