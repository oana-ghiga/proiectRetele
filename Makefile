run: build
	./bin/main
build:
	gcc src/main.c -w -lSDL2 -lSDL2_image -o bin/main
