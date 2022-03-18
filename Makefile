default: chip8

chip8.o: chip8.c
	gcc -g -c chip8.c -o chip8.o  -c -I/opt/homebrew/opt/sdl2/include/

chip8: chip8.o
	gcc chip8.o -o chip8 -L/opt/homebrew/lib -lSDL2

cross:
	x86_64-w64-mingw32-gcc chip8.c -I/tmp/sdl2-win64/include/ -L/tmp/sdl2-win64/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -o chip8.exe

clean:
	rm -rf *.o chip8 chip8.exe
