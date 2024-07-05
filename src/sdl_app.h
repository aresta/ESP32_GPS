#pragma once

#define SDL_MAIN_HANDLED

#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>

void Run();

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

	auto _sdl = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(_sdl, 255, 0, 0, 0);
	SDL_RenderClear(_sdl);

	SDL_RenderPresent(_sdl);

	if (_sdl == NULL) {
		fprintf(stderr, "ERROR: can't create renderer: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	auto start_time = std::chrono::high_resolution_clock::now();

    Run();

    auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

    std::cout << time / std::chrono::milliseconds(1) << "ms to run.\n";

	do
	{
		SDL_Delay(10);
		SDL_PollEvent(&event);
	} 
	while (event.type != SDL_QUIT);

	SDL_Quit();

	return 0;
}