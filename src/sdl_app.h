#pragma once

#define SDL_MAIN_HANDLED

#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>

SDL_Renderer* _sdl;

void Setup();

void Loop();

void delay(uint32_t msec)
{
    SDL_Delay(msec);
}

bool ReadInput(int pin)
{
	const Uint8* state = SDL_GetKeyboardState(nullptr);

	switch (pin)
	{
		case SELECT_BUTTON:
			return state[SDL_SCANCODE_S];

		case UP_BUTTON:
			return state[SDL_SCANCODE_UP];
	
		case DOWN_BUTTON:
			return state[SDL_SCANCODE_DOWN];

		case LEFT_BUTTON:
			return state[SDL_SCANCODE_LEFT];

		case RIGHT_BUTTON:
			return state[SDL_SCANCODE_RIGHT];

		default:
			return false;
	}
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
	SDL_Event event;

    auto window = SDL_CreateWindow(
		"SDL Maps Renderer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		TFT_WIDTH, 
        TFT_HEIGHT, 
		SDL_WINDOW_RESIZABLE ); //| SDL_WINDOW_METAL

	if (window == NULL) {
		fprintf(stderr, "ERROR: can't create window: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	_sdl = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (_sdl == NULL) {
		fprintf(stderr, "ERROR: can't create renderer: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	// auto start_time = std::chrono::high_resolution_clock::now();

    Setup();

	SDL_RenderPresent(_sdl);

    // auto end_time = std::chrono::high_resolution_clock::now();
	// auto time = end_time - start_time;

    //std::cout << time / std::chrono::milliseconds(1) << "ms to run.\n";

	do {
		Loop();

		SDL_Delay(10);
		SDL_PollEvent(&event);
		SDL_RenderPresent(_sdl);
	} 
	while (event.type != SDL_QUIT);

	SDL_DestroyRenderer(_sdl);
	SDL_Quit();

	return 0;
}