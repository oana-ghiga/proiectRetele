run: build
	./bin/main 127.0.0.1 1234
build:
	gcc src/main.c -w -lSDL2 -lSDL2_image -o bin/main
