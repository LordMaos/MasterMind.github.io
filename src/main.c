/*
** NOM    : SEPKAP YEPMO
** PRENOM : YVAN RYAN
*/
// Using SDL, SDL_image, standard io, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

// Buttons positions to facilitate mouse positon checking
#define TOP_BUTTON_X (max(0, SCREEN_WIDTH - BUTTON_WIDTH) / 2)
#define TOP_BUTTON_Y SCREEN_HEIGHT / 4
#define BOTTOM_BUTTON_X (max(0, SCREEN_WIDTH - BUTTON_WIDTH) / 2)
#define BOTTOM_BUTTON_Y 3 * SCREEN_HEIGHT / 4

// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

// Spacing constants
const int PADDING = 5;
const int MARGIN = 25;
const int CIRCLE_RADIUS = 10;

// Colors
const int BG_GREY_COLOR = 0x80;
const int CIRCLE_PERIMETER_COLOR = 0xFF;
const int ENTIRELY_CORRECT_COLOR = 0xFF;
const int MID_CORRECT_COLOR = 0x00;

//Button constants
const int BUTTON_WIDTH = 200;
const int BUTTON_HEIGHT = 50;


// Game constants
const int NB_TRIALS = 10;
const int CODE_LENGTH = 4;
const int NB_COLORS = 4;

// Game buttons
const SDL_Keycode verifyButtonCodes[2] = { SDLK_RETURN, SDLK_KP_ENTER };


// Color sprites
enum ColorSprite
{
	COLOR_0,
	COLOR_1,
	COLOR_2,
	COLOR_3,
	COLOR_DEFAULT,
	COLOR_TOTAL
};

// Button
enum Texture
{
	TEXTURE_EXIT_GAME,
	TEXTURE_NEW_GAME,
	TEXTURE_LOAD_GAME,
	TEXTURE_GAME_OVER,
	TEXTURE_WIN,
	TEXTURE_RULES,
	TEXTURE_COLORS,
	TEXTURES_TOTAL
};

// Game state
enum GameFrame
{
	HOME,
	RULES,
	PLAY,
	GAME_LOST,
	GAME_WON
};

struct GameState
{
	enum GameFrame gameFrame;
	uint8_t* secretCode;
	uint8_t* codeGuess;
	int currentTrial;
};

// Indicates whether mouse is inside a guess circle
struct MousePositionCheck
{
	bool isInside;
	uint8_t circleIndex;
};

//The window we'll be rendering to
SDL_Window* window = NULL;

//The window renderer
SDL_Renderer* renderer = NULL;

// Textures
SDL_Texture* textures[TEXTURES_TOTAL];

// Colors clips
SDL_Rect colorSpriteClips[COLOR_TOTAL - 1];
int colorTextureWidth, colorTextureHeight; ///////////////////////////////

// Starts up SDL and creates window
bool init();

// Frees media and shuts down SDL
void closeApp(struct GameState* gameState);

// Loads images as textures
bool loadTextures();

// Function to draw a circle
// Adapted from https://en.wikipedia.org/w/index.php?title=Midpoint_circle_algorithm&oldid=889172082#C_example
// The objective of the algorithm is to find a path through the pixel grid
// using pixels which are as close as possible to solutions of x2 + y2 = r2
// cx : x coord of centre
// cy : y coord of centre
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius);

// Function to fill a circle
void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius);

// Game frame functions
void setButton(enum Texture texture, bool top);
void setHomeFrame();
void setRulesFrame();
void setBoardFrame();
void setGameOverFrame();
void setVictoryFrame();

// Game events
void initializeGameState(struct GameState* gameState);
void handleEvent(SDL_Event* e, struct GameState* gameState);
void handleMouseDown(int x, int y, struct GameState* gameState);
void handleKeyDown(SDL_Keycode keyCode, struct GameState* gameState);
struct MousePositionCheck checkClickedCircle(int x, int y, int currentTrial, int codeLength);
void changeCircleColor(int currentTrial, int circleIndex, uint8_t* codeGuess);

// Secret code utilities
uint8_t* generateCode(int codeLength);
void initializeCodeGuess(uint8_t* codeGuess);
bool verifyCode(uint8_t* secretCode, uint8_t* codeGuess);
void freeCodes(uint8_t* secretCode, uint8_t* codeGuess);

/*************************************************************************/
							/* App management */
/*************************************************************************/
bool init()
{
	// Initialization flag
	bool success = true;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		// Create window
		window = SDL_CreateWindow("Mastermind", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// Create renderer for window
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				// Initialize renderer color
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				// Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}

				for (int i = 0; i < TEXTURES_TOTAL; ++i) {
					textures[i] = NULL;
				}
			}
		}
	}
	return success;
}

bool loadTextures()
{
	// Loading success flag
	bool success = true;

	// Load textures
	const char* texturesPath[TEXTURES_TOTAL];
	texturesPath[TEXTURE_EXIT_GAME] = "./assets/exit.png";
	texturesPath[TEXTURE_NEW_GAME] = "./assets/new_game.png";
	texturesPath[TEXTURE_LOAD_GAME] = "./assets/load_game.png";
	texturesPath[TEXTURE_GAME_OVER] = "./assets/game_over.png";
	texturesPath[TEXTURE_WIN] = "./assets/win.png";
	texturesPath[TEXTURE_RULES] = "./assets/rules.png";
	texturesPath[TEXTURE_COLORS] = "./assets/colors.png";

	for (int i = 0; i < TEXTURES_TOTAL; ++i) {
		//Load image at specified path
		SDL_Surface* loadedSurface = IMG_Load(texturesPath[i]);
		if (loadedSurface == NULL)
		{
			printf("Unable to load image %s! SDL_image Error: %s\n", texturesPath[i], IMG_GetError());
			success = false;
		}
		else
		{
			if (i == TEXTURE_COLORS) {
				// Color key image
				SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
			}

			// Create texture from surface pixels
			textures[i] = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			if (textures[i] == NULL)
			{
				printf("Unable to create texture from %s! SDL Error: %s\n", texturesPath[i], SDL_GetError());
				success = false;
			}

			if (i == TEXTURE_COLORS) {
				colorTextureWidth = loadedSurface->w;
				colorTextureHeight = loadedSurface->h;

				// Set color 0 sprite
				colorSpriteClips[0].x = 0;
				colorSpriteClips[0].y = 0;
				colorSpriteClips[0].w = 100;
				colorSpriteClips[0].h = 100;

				// Set color 1 sprite
				colorSpriteClips[1].x = 100;
				colorSpriteClips[1].y = 0;
				colorSpriteClips[1].w = 100;
				colorSpriteClips[1].h = 100;

				// Set color 2 sprite
				colorSpriteClips[2].x = 0;
				colorSpriteClips[2].y = 100;
				colorSpriteClips[2].w = 100;
				colorSpriteClips[2].h = 100;

				// Set color 3 sprite
				colorSpriteClips[3].x = 100;
				colorSpriteClips[3].y = 100;
				colorSpriteClips[3].w = 100;
				colorSpriteClips[3].h = 100;
			}

			// Get rid of old loaded surface
			SDL_FreeSurface(loadedSurface);
		}
	}

	return success;
}

void closeApp(struct GameState* gameState)
{
	if (gameState != NULL) {
		freeCodes(gameState->secretCode, gameState->codeGuess);
	}

	// Free loaded image
	for (int i = 0; i < TEXTURES_TOTAL; ++i) {
		SDL_DestroyTexture(textures[i]);
		textures[i] = NULL;
	}

	// Destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	// Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}
/*************************************************************************/
			         /* Drawing utilities */
/*************************************************************************/
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
	int x = radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		SDL_RenderDrawPoint(renderer, cx + x, cy + y);
		SDL_RenderDrawPoint(renderer, cx + y, cy + x);
		SDL_RenderDrawPoint(renderer, cx - y, cy + x);
		SDL_RenderDrawPoint(renderer, cx - x, cy + y);
		SDL_RenderDrawPoint(renderer, cx - x, cy - y);
		SDL_RenderDrawPoint(renderer, cx - y, cy - x);
		SDL_RenderDrawPoint(renderer, cx + y, cy - x);
		SDL_RenderDrawPoint(renderer, cx + x, cy - y);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}

	// Update screen
	SDL_RenderPresent(renderer);
}

void fillCircle(SDL_Renderer* renderer, int cx, int cy, int radius)
{
	int x = radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
		SDL_RenderDrawLine(renderer, cx - y, cy + x, cx + y, cy + x);
		SDL_RenderDrawLine(renderer, cx - x, cy - y, cx + x, cy - y);
		SDL_RenderDrawLine(renderer, cx - y, cy - x, cx + y, cy - x);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
	}

	// Update screen
	SDL_RenderPresent(renderer);
}

void setButton(enum Texture texture, bool top)
{
	SDL_Rect renderRect;
	renderRect.w = BUTTON_WIDTH;  // The width of the button
	renderRect.h = BUTTON_HEIGHT; // The height of the button
	if (top) {
		renderRect.x = TOP_BUTTON_X;  // The x coordinate
		renderRect.y = TOP_BUTTON_Y;  // The y coordinate
	}
	else {
		renderRect.x = BOTTOM_BUTTON_X;
		renderRect.y = BOTTOM_BUTTON_Y;
	}
	SDL_RenderCopy(renderer, textures[texture], NULL, &renderRect);
}
/*************************************************************************/
				/* Game frame functions */
/*************************************************************************/
void setHomeFrame()
{
	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	// Render textures to screen
	setButton(TEXTURE_NEW_GAME, true);
	setButton(TEXTURE_EXIT_GAME, false);

	//Update screen
	SDL_RenderPresent(renderer);
}

void setRulesFrame()
{
	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);

	// Render textures to screen
	SDL_Rect renderRect;
	renderRect.x = max(0, SCREEN_WIDTH - 500) / 2;
	renderRect.y = 0;
	renderRect.w = 500;
	renderRect.h = BOTTOM_BUTTON_Y - MARGIN;
	SDL_RenderCopy(renderer, textures[TEXTURE_RULES], NULL, &renderRect);

	setButton(TEXTURE_LOAD_GAME, false);

	// Update screen
	SDL_RenderPresent(renderer);
}

void setBoardFrame()
{
	// Clear screen
	SDL_SetRenderDrawColor(renderer, BG_GREY_COLOR, BG_GREY_COLOR, BG_GREY_COLOR, 0xFF);
	SDL_RenderClear(renderer);

	int paneWidth = (2 * CIRCLE_RADIUS + PADDING) * CODE_LENGTH;
	int paneHeight = (2 * CIRCLE_RADIUS + PADDING) * NB_TRIALS;
	if (SCREEN_WIDTH < paneWidth || SCREEN_HEIGHT < paneHeight) {
		printf("Not enough space on screen to draw the board\n");
		closeApp(NULL);
	}

	SDL_SetRenderDrawColor(renderer, CIRCLE_PERIMETER_COLOR, CIRCLE_PERIMETER_COLOR, CIRCLE_PERIMETER_COLOR, 0xFF);
	for (int i = 0; i < NB_TRIALS; ++i) {
		for (int j = 0; j < CODE_LENGTH; ++j) {
			// Left pane
			int cx = ( (SCREEN_WIDTH - MARGIN) / 2 ) -  CIRCLE_RADIUS - j * (2 * CIRCLE_RADIUS + PADDING);
			int cy = MARGIN + CIRCLE_RADIUS + i * (2 * CIRCLE_RADIUS + PADDING);
			drawCircle(renderer, cx, cy, CIRCLE_RADIUS);

			// Right pane
			cx = SCREEN_WIDTH - cx;
			drawCircle(renderer, cx, cy, CIRCLE_RADIUS);
		}
	}

	setButton(TEXTURE_NEW_GAME, false);

	// Update screen
	SDL_RenderPresent(renderer);
}

void setGameOverFrame()
{
	// Render textures to screen
	SDL_Rect renderRect;
	renderRect.x = max(0, SCREEN_WIDTH - 400) / 2;
	renderRect.y = MARGIN;
	renderRect.w = 400;
	renderRect.h = min(400, BOTTOM_BUTTON_Y - MARGIN);
	SDL_RenderCopy(renderer, textures[TEXTURE_GAME_OVER], NULL, &renderRect);

	// Update screen
	SDL_RenderPresent(renderer);
}

void setVictoryFrame()
{
	// Render textures to screen
	SDL_Rect renderRect;
	renderRect.x = max(0, SCREEN_WIDTH - 400) / 2;
	renderRect.y = MARGIN;
	renderRect.w = 400;
	renderRect.h = min(400, BOTTOM_BUTTON_Y - MARGIN);
	SDL_RenderCopy(renderer, textures[TEXTURE_WIN], NULL, &renderRect);

	// Update screen
	SDL_RenderPresent(renderer);
}
/*************************************************************************/
					/* Game events */
/*************************************************************************/
void initializeGameState(struct GameState* gameState)
{
	if (gameState->codeGuess == NULL) {
		gameState->codeGuess = malloc(CODE_LENGTH * sizeof(uint8_t));
	}
	initializeCodeGuess(gameState->codeGuess);
	gameState->currentTrial = 0;
}

void handleEvent(SDL_Event* e, struct GameState* gameState)
{
	if (e->type == SDL_MOUSEBUTTONDOWN) {
		// Get mouse position
		int x, y;
		SDL_GetMouseState(&x, &y);
		handleMouseDown(x, y, gameState);
	}
	else if (e->type == SDL_KEYDOWN && gameState->gameFrame == PLAY) {
		handleKeyDown(e->key.keysym.sym, gameState);
	}
}

void handleMouseDown(int x, int y, struct GameState* gameState)
{
	switch (gameState->gameFrame) {
		case HOME:
			if (x >= TOP_BUTTON_X && x <= TOP_BUTTON_X + BUTTON_WIDTH && y >= TOP_BUTTON_Y && y <= TOP_BUTTON_Y + BUTTON_HEIGHT) {
				setRulesFrame();
				gameState->gameFrame = RULES;
			}
			else if (x >= BOTTOM_BUTTON_X && x <= BOTTOM_BUTTON_X + BUTTON_WIDTH && y >= BOTTOM_BUTTON_Y && y <= BOTTOM_BUTTON_Y + BUTTON_HEIGHT) {
				closeApp(gameState);
			}
			break;
		case RULES:
			if (x >= BOTTOM_BUTTON_X && x <= BOTTOM_BUTTON_X + BUTTON_WIDTH && y >= BOTTOM_BUTTON_Y && y <= BOTTOM_BUTTON_Y + BUTTON_HEIGHT) {
				setBoardFrame();
				gameState->gameFrame = PLAY;
				if (gameState->secretCode != NULL) free(gameState->secretCode);
				gameState->secretCode = generateCode(CODE_LENGTH);
			}
			break;
		case PLAY:
			if (x >= BOTTOM_BUTTON_X && x <= BOTTOM_BUTTON_X + BUTTON_WIDTH && y >= BOTTOM_BUTTON_Y && y <= BOTTOM_BUTTON_Y + BUTTON_HEIGHT) {
				setRulesFrame();
				gameState->gameFrame = RULES;
				initializeGameState(gameState);
			}
			else {
				struct MousePositionCheck mousePositionCheck = checkClickedCircle(x, y, gameState->currentTrial, CODE_LENGTH);
				if (mousePositionCheck.isInside) changeCircleColor(gameState->currentTrial, mousePositionCheck.circleIndex, gameState->codeGuess);
			}
			break;
		case GAME_LOST:
		case GAME_WON:
			if (x >= BOTTOM_BUTTON_X && x <= BOTTOM_BUTTON_X + BUTTON_WIDTH && y >= BOTTOM_BUTTON_Y && y <= BOTTOM_BUTTON_Y + BUTTON_HEIGHT) {
				setRulesFrame();
				gameState->gameFrame = RULES;
			}
			break;
		default:
			break;
	}
}

void handleKeyDown(SDL_Keycode keyCode, struct GameState* gameState)
{

	if (keyCode == verifyButtonCodes[0] || keyCode == verifyButtonCodes[1]) {
		for (int i = 0; i < CODE_LENGTH; ++i) {
			if (gameState->codeGuess[i] == COLOR_DEFAULT) return;
		}
		bool codeFound = verifyCode(gameState->secretCode, gameState->codeGuess, gameState->currentTrial);
		if (!codeFound && gameState->currentTrial == NB_TRIALS - 1) {
			gameState->gameFrame = GAME_LOST;
			setGameOverFrame();
			initializeGameState(gameState);
		}
		else if (codeFound) {
			setVictoryFrame();
			gameState->gameFrame = GAME_WON;
			initializeGameState(gameState);
		}
		else {
			++gameState->currentTrial;
		}
	}
}

struct MousePositionCheck checkClickedCircle(int x, int y, int currentTrial, int codeLength)
{
	struct MousePositionCheck mousePositionCheck = {false, 0};

	int cy = MARGIN + CIRCLE_RADIUS + ((NB_TRIALS - 1) - currentTrial) * (2 * CIRCLE_RADIUS + PADDING);
	if (y >= cy - CIRCLE_RADIUS && y <= cy + CIRCLE_RADIUS) {

		int rightEdge = (SCREEN_WIDTH - MARGIN) / 2;
		int leftEdge = rightEdge - ((PADDING + 2 * CIRCLE_RADIUS) * (codeLength - 1) + 2 * CIRCLE_RADIUS);

		if (x >= leftEdge && x <= rightEdge) {
			int d = rightEdge - x;
			int j = d / (2 * CIRCLE_RADIUS + PADDING);
			int cx = ((SCREEN_WIDTH - MARGIN) / 2) - CIRCLE_RADIUS - j * (2 * CIRCLE_RADIUS + PADDING);

			if (x >= cx - CIRCLE_RADIUS && x <= cx + CIRCLE_RADIUS) {
				mousePositionCheck.isInside = true;
				mousePositionCheck.circleIndex = j;
			}
		}
	}

	return mousePositionCheck;
}

void changeCircleColor(int currentTrial, int circleIndex, uint8_t* codeGuess)
{
	int realIndex = CODE_LENGTH - 1 - circleIndex;
	codeGuess[realIndex] = codeGuess[realIndex] == COLOR_DEFAULT ? COLOR_0 : (codeGuess[realIndex] + 1) % CODE_LENGTH;

	int cx = ((SCREEN_WIDTH - MARGIN) / 2) - CIRCLE_RADIUS - circleIndex * (2 * CIRCLE_RADIUS + PADDING);
	int cy = MARGIN + CIRCLE_RADIUS + ((NB_TRIALS - 1) - currentTrial) * (2 * CIRCLE_RADIUS + PADDING);

	SDL_Rect renderQuad = { cx - CIRCLE_RADIUS, cy - CIRCLE_RADIUS, 2 * CIRCLE_RADIUS, 2 * CIRCLE_RADIUS };
	const SDL_Rect clip = colorSpriteClips[codeGuess[realIndex]];

	// Render next color to screen
	SDL_RenderCopy(renderer, textures[TEXTURE_COLORS], &clip, &renderQuad);
	SDL_RenderPresent(renderer);
}
/*************************************************************************/
					/* Secret code utilities */
/*************************************************************************/
uint8_t* generateCode(int codeLength)
{
	srand(time(NULL));   // Initialization
	uint8_t* secretCode = malloc(codeLength * sizeof(uint8_t));
	if (secretCode == NULL) {
		closeApp(NULL);
	}
	else {
		for (int i = 0; i < codeLength; ++i) {
			secretCode[i] = rand() % NB_COLORS;
			printf("%d\n", secretCode[i]);
		}
	}
	return secretCode;
}

void initializeCodeGuess(uint8_t* codeGuess)
{
	for (int i = 0; i < CODE_LENGTH; ++i) {
		codeGuess[i] = COLOR_DEFAULT;
	}
}

bool verifyCode(uint8_t* secretCode, uint8_t* codeGuess, int currentTrial)
{
	bool codeFound = true;
	int entirelyCorrect = 0;
	int midCorrect = 0;

	for (int i = 0; i < CODE_LENGTH; ++i) {
		for (int j = 0; j < CODE_LENGTH; ++j) {
			if (secretCode[i] == codeGuess[j] && i == j) {
				++entirelyCorrect;
				codeGuess[j] = COLOR_DEFAULT;
				break;
			}
			else if (secretCode[i] == codeGuess[j]) {
				++midCorrect;
				codeGuess[j] = COLOR_DEFAULT;
				break;
			}
		}
	}

	codeFound = entirelyCorrect == CODE_LENGTH;

	if (!codeFound) {
		int toColor = entirelyCorrect + midCorrect;
		for (int i = 0; i < toColor; ++i) {
			int cx = ((SCREEN_WIDTH + MARGIN) / 2) + CIRCLE_RADIUS + i * (2 * CIRCLE_RADIUS + PADDING);
			int cy = MARGIN + CIRCLE_RADIUS + ((NB_TRIALS - 1) - currentTrial) * (2 * CIRCLE_RADIUS + PADDING);

			if (entirelyCorrect > 0) {
				SDL_SetRenderDrawColor(renderer, ENTIRELY_CORRECT_COLOR, ENTIRELY_CORRECT_COLOR, ENTIRELY_CORRECT_COLOR, 0xFF);
				fillCircle(renderer, cx, cy, CIRCLE_RADIUS);
				--entirelyCorrect;
			}
			else if (midCorrect > 0) {
				SDL_SetRenderDrawColor(renderer, MID_CORRECT_COLOR, MID_CORRECT_COLOR, MID_CORRECT_COLOR, 0xFF);
				fillCircle(renderer, cx, cy, CIRCLE_RADIUS);
				--midCorrect;
			}
		}
	}

	initializeCodeGuess(codeGuess);

	return codeFound;
}

void freeCodes(uint8_t* secretCode, uint8_t* codeGuess) {
	if (secretCode != NULL) {
		free(secretCode);
	}
	if (codeGuess != NULL) {
		free(codeGuess);
	}
}
/*************************************************************************/



/////////////////////////////////MAIN/////////////////////////////////////
int main(int argc, char* args[])
{
	// Game current state
	struct GameState gameState = { HOME, NULL, NULL, 0};

	// Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load textures
		if (!loadTextures())
		{
			printf("Failed to load textures!\n");
		}
		else
		{
			// Main loop flag
			bool quit = false;

			// Event handler
			SDL_Event e;

			initializeGameState(&gameState);

			setHomeFrame();

			// While application is running
			while (!quit)
			{
				// Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					else
					{
						handleEvent(&e, &gameState);
					}
				}
			}
		}
	}

	closeApp(&gameState);

	return 0;
}
