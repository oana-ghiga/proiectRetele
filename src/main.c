/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
// for initializing and shutdown functions
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define false 0
#define true 1

#define MAT_SIZE 9
#define BTN_SIZE 62
#define X_OFFSET 200
#define Y_OFFSET 185

// definitions for images
#define IMG_BLACK "img/black.png"
#define IMG_LOBBY "img/lobby.png"
#define IMG_PASS_BUTTON "img/pass_button.png"
#define IMG_START_BUTTON "img/start_button.png"
#define IMG_BACKGROUND "img/start.png"
#define IMG_TABLA_GO "img/tabla_go.png"
#define IMG_WHITE "img/white.png"

int mat[] = {1, 0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 2, 2, 1, 0, 1, 0, 0, 1, 1, 1, 1, 2, 0, 2, 2, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1, 1, 2, 0, 0, 0, 1, 2, 0, 2, 0, 2, 0, 0, 2, 1, 0, 1, 2, 1, 1, 0, 1, 0, 2, 1, 1, 0, 0, 2, 2, 2, 2, 0, 1, 2, 0, 1, 0, 1, 2};
//enum for game states
enum states {
	STATE_MENU,
	STATE_LOBBY,
	STATE_GAME
} state;

// enum for errors
enum errors {
	err_sdl = 1000,
	err_window,
	invalid_state,
	invalid_arg,
	err_socket,
	err_connect
};

typedef struct {
	int r, g, b, a;
} color_t;

typedef struct {
	SDL_Rect btn_rect;

	color_t color;
	char *imagePath;

	char isActive;
	char isPressed;
} button_t;

void initButtons(button_t *btns);
static void button_process_event(button_t *btn, const SDL_Event *ev);
void drawButton(button_t *btn, SDL_Renderer *r);
char checkButton(button_t *btn);
void displayMainBg(SDL_Renderer* renderer);
void displayLobbyBg(SDL_Renderer* renderer);
void displayGameBg(SDL_Renderer* renderer);
void displayPieces(int *mat, button_t* btns, SDL_Renderer* renderer);

//Screen dimension constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;


// expected start: client <srv_ip> <srv_port>
int main( int argc, char* args[] ) {
	char isGameOver = 0;
	state = STATE_MENU;
	int port;
	int srvSocketD;
	struct sockaddr_in servStruct;
	char respBuffer[10];
	int respTmp;

	// validate input 
	if (argc != 3) {
		printf("Usage: %s <server_ip> <server_port>", args[0]);
		exit(invalid_arg);
	}

	// socket stuff
	port = atoi(args[2]);

	if ((srvSocketD = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
      perror ("Eroare la socket().\n");
      return err_socket;
    }

	servStruct.sin_addr.s_addr = inet_addr(args[1]);
	servStruct.sin_port = htons(port);
	servStruct.sin_family = AF_INET;

	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	// init start button
	button_t start_button = {
        .color = { .r = 0, .g = 0, .b = 0, .a = 0, },
        .btn_rect = { .x = SCREEN_WIDTH/2-125, .y = 800, .w = 250, .h = 100 },
    };
	start_button.imagePath = malloc(strlen(IMG_START_BUTTON));
	start_button.isActive = true;
	strcpy(start_button.imagePath, IMG_START_BUTTON);

	button_t intersectionsBtns[MAT_SIZE * MAT_SIZE];
	initButtons(intersectionsBtns);

	//Initialize SDL
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(err_sdl);
	} else {
		//Create window
		window = SDL_CreateWindow( "Go Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			exit(err_window);
		}
		//Get window surface
		screenSurface = SDL_GetWindowSurface( window );

		// Create renderer
		SDL_Renderer* renderer = NULL;
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if(!renderer) {   // renderer creation may fail too
			fprintf(stderr, "create renderer failed: %s\n", SDL_GetError());
			return 1;
		}
		
		// main event handling loop
		SDL_Event evt; 
		while(!isGameOver ) { 
			SDL_ClearError();
			while( SDL_PollEvent( &evt ) ) {
				if(evt.type == SDL_QUIT ||
					(evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE) ||
					(evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)) {
					isGameOver = true; 
				}
				switch (state) {
					case STATE_MENU:
						button_process_event(&start_button, &evt);
						break;
					case STATE_GAME:
						for (int i = 0; i < MAT_SIZE*MAT_SIZE; i++) {
							button_process_event(&intersectionsBtns[i], &evt);
						}
						break;
					default: 
						break;
				}
			} 
			SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
			SDL_RenderClear(renderer);
			switch (state) {
				case STATE_MENU:
					displayMainBg(renderer);

					drawButton(&start_button, renderer);
					if(checkButton(&start_button) == true) {
						printf("start button pressed\n");
						state = STATE_LOBBY;   // state change - button will not be drawn anymore
					}
					break;
				case STATE_LOBBY:
					displayLobbyBg(renderer);

					if (connect(srvSocketD, (struct sockaddr*)&servStruct, sizeof(servStruct)) == -1) {
						perror("Error connecting to server!");
						exit(err_connect);
					}
					state = STATE_GAME;

				case STATE_GAME:
					displayGameBg(renderer);
					
					bzero(mat, sizeof(mat));
					read(srvSocketD, mat, sizeof(mat));
					displayPieces(mat, intersectionsBtns, renderer);
					SDL_RenderPresent(renderer);

					for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) { 
						// drawButton(&intersectionsBtns[i], renderer);
						if (checkButton(&intersectionsBtns[i]) == true) {
							bzero(respBuffer, sizeof(respBuffer));
							printf("Intersection button pressed\n Line: %d\t Column: %d\n", i/MAT_SIZE, i%MAT_SIZE);
							sprintf(respBuffer, "%d%d", i/MAT_SIZE+1, i%MAT_SIZE+1);
							respTmp = atoi(respBuffer);
							respTmp = htonl(respTmp);
							write(srvSocketD, &respTmp, sizeof(respTmp));
						}
					}

					read(srvSocketD, mat, sizeof(mat));
					SDL_RenderClear(renderer);
					displayGameBg(renderer);
					displayPieces(mat, intersectionsBtns, renderer);

					break;
				default:
					exit(invalid_state);
					break;
			}
			SDL_Delay(200);
			SDL_RenderPresent(renderer);
		}
	}

	//Destroy window
	SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

void initButtons(button_t *btns) {
	int xOffset = 5;
	int yOffset = 5;
	color_t color = { .r = 255, .g=0, .b=0, .a=255};
	SDL_Rect poz;
	poz.w = BTN_SIZE;
	poz.h = BTN_SIZE;

	int row, col;
	for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
		col = i/MAT_SIZE;
		row = i%MAT_SIZE;
		btns[i].color = color;
		poz.x = X_OFFSET + row*(xOffset+BTN_SIZE);
		poz.y = Y_OFFSET + col*(yOffset+BTN_SIZE);
		btns[i].btn_rect = poz;
		btns[i].isPressed = false;
		btns[i].isActive = true;
		btns[i].imagePath = malloc(strlen(IMG_START_BUTTON));
	}
}

static void button_process_event(button_t *btn, const SDL_Event *ev) {
    // react on mouse click within button rectangle by setting 'pressed'
    if((ev->type == SDL_MOUSEBUTTONDOWN) && (btn->isActive == true)) {
        if(ev->button.button == SDL_BUTTON_LEFT &&
                ev->button.x >= btn->btn_rect.x &&
                ev->button.x <= (btn->btn_rect.x + btn->btn_rect.w) &&
                ev->button.y >= btn->btn_rect.y &&
                ev->button.y <= (btn->btn_rect.y + btn->btn_rect.h)) {
            btn->isPressed = true;
        }
    }
}

void drawButton(button_t *btn, SDL_Renderer *r) {
	// SDL_SetRenderDrawColor(r, btn->color.r, btn->color.g, btn->color.b, btn->color.a);
    // SDL_RenderFillRect(r, &btn->btn_rect);

	if (strlen(btn->imagePath) > 0) {
		SDL_Rect btn_rect = btn->btn_rect;

		SDL_Texture *texture = IMG_LoadTexture(r, btn->imagePath);
		SDL_RenderCopy(r, texture, NULL, &btn_rect);
	}
}

char checkButton(button_t *btn) {
	if (btn->isPressed) {
		btn->isPressed = false;
		return true;
	}
	return false;
}

void displayMainBg(SDL_Renderer* renderer){

	SDL_Texture *texture = IMG_LoadTexture(renderer, IMG_BACKGROUND);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void displayLobbyBg(SDL_Renderer* renderer){
	SDL_Texture *texture = IMG_LoadTexture(renderer, IMG_LOBBY);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void displayGameBg(SDL_Renderer* renderer){
	SDL_Texture *texture = IMG_LoadTexture(renderer, IMG_TABLA_GO);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void displayPieces(int *mat, button_t* btns, SDL_Renderer* renderer) {
	for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
		if (mat[i] == 1) {
			// draw button with black
			free(btns[i].imagePath);
			btns[i].isActive = false;
			btns[i].imagePath = malloc(strlen(IMG_BLACK));
			strcpy(btns[i].imagePath, IMG_BLACK);
			drawButton(&btns[i], renderer);
		} else if (mat[i] == 2) {
			// draw button with white
			free(btns[i].imagePath);
			btns[i].isActive = false;
			btns[i].imagePath = malloc(strlen(IMG_WHITE));
			strcpy(btns[i].imagePath, IMG_WHITE);
			drawButton(&btns[i], renderer);
		}
	}
}
// 1 negru ; 2 alb
// <- server: tabla
// -> client: coordonate 43 linia 4 coloana 3 un singur int
// <- server: matricea 
