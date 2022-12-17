/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
// for initializing and shutdown functions
#include <SDL2/SDL.h>

// for rendering images and graphics on screen
#include <SDL2/SDL_image.h>

// for using SDL_Delay() functions
#include <SDL2/SDL_timer.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>


#define false 0
#define true 1

// definitions for images
#define IMG_BACKGROUND "img/start.png"

// enum for errors
enum errors {
	error_sdl = 1000,
	error_window
};



void displayMain(SDL_Surface* screenSurface, SDL_Window* gameWindow);
void displayLobby();
void displayGame();

//Screen dimension constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

int main( int argc, char* args[] )
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(error_sdl);
	}
	else {
		//Create window
		window = SDL_CreateWindow( "Go Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			exit(error_window);
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface( window );

			displayMain(screenSurface, window);
            
            //Hack to get window to stay up
            SDL_Event e; int quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }
		}
	}

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

void displayMain(SDL_Surface* screenSurface, SDL_Window* gameWindow){
	SDL_Rect bg;

	bg.x = 0;
	bg.y = 0;

	SDL_Surface* play = NULL;
	play = IMG_Load(IMG_BACKGROUND);
	SDL_BlitSurface(play, NULL, screenSurface, &bg);

	SDL_UpdateWindowSurface(gameWindow);
	printf("should draw\n");
	free(play);
}

void displayLobby(){

}

void displayGame(){

}
