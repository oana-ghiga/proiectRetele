#include <SDL.h>
#include <stdio.h>
#include <SDL_image.h>
#include"data.h"



const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 700;

int main() {
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		// End the program
		return 1;
	} 

	window = SDL_CreateWindow( "Go game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN );
	return 0;
}