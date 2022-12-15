#pragma once

#include <chrono>
#include <SDL.h>

#define MOUSE_INPUTS 6
#define KEYBOARD_INPUTS 1024

struct Engine;

struct Input {
private:
	bool lastMouse[MOUSE_INPUTS]{ false };
	bool mouse[MOUSE_INPUTS]{ false };
	bool lastKeys[KEYBOARD_INPUTS]{ false };
	bool keys[KEYBOARD_INPUTS]{ false };
	float mouseX = 0, mouseY = 0;
	float scroll = 0;

	friend struct SDLWindowEngine;
	void update() {
		scroll = 0;
		uint32_t i = 0;
		for (i; i < MOUSE_INPUTS; i++) { lastMouse[i] = mouse[i]; }
		for (i = 0; i < KEYBOARD_INPUTS; i++) { lastKeys[i] = keys[i]; }
	}

public:
	bool isKeyPressed(uint32_t key) { return key < KEYBOARD_INPUTS ? (keys[key] && !lastKeys[key]) : false; }
	bool isKeyDown(uint32_t key) { return key < KEYBOARD_INPUTS ? keys[key] : false; }
	bool isKeyReleased(uint32_t key) { return key < KEYBOARD_INPUTS ? (!keys[key] && lastKeys[key]) : false; }
	bool isMouseButtonPressed(uint32_t key) { return key < MOUSE_INPUTS ? (mouse[key] && !lastMouse[key]) : false; }
	bool isMouseButtonDown(uint32_t key) { return key < MOUSE_INPUTS ? mouse[key] : false; }
	bool isMouseButtonReleased(uint32_t key) { return key < MOUSE_INPUTS ? (!mouse[key] && lastMouse[key]) : false; }
	void pushMouseButtonEvent(SDL_Event* event, bool pressed) {
		uint8_t button = event->button.button;
		if (button < MOUSE_INPUTS) { mouse[button] = pressed; }
	}
	void pushMouseWheelEvent(SDL_Event* event) {
		// TODO
	}
	void pushMouseMovementEvent(SDL_Event* event) {
		// TODO
	}
	void pushKeyEvent(SDL_Event* event, bool pressed) {
		uint32_t key = event->key.keysym.scancode;
		if (key < KEYBOARD_INPUTS) { keys[key] = pressed; }
		else { printf_s("Key was pressed or realeased that is not saved in scancode input array! (%i)\n", key); }
	}
};

struct SDLWindowEngine {
private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Surface* surface = nullptr;
	SDL_Texture* tex = nullptr;

public:
	friend struct Engine;

	// Data to be used by program
	SDL_PixelFormat* format;
	Input input;
	int width, height;
	bool isRunning;

	// Constructor
	SDLWindowEngine() {
		isRunning = true;
		window = NULL;
		renderer = NULL;
	}

	// timing function
	inline static const uint64_t getTime() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }

	// Main function that calls all the functions for the program
	int execute(const char* title, int width, int height) {
		SDL_Event event;
		if (!init(title, width, height)) { return -1; }
		while (isRunning) {
			input.update(); // moves current inputs to last frames inputs so we can check if input has been released that frame
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN: { input.pushKeyEvent(&event, false); break; }
				case SDL_KEYUP: { input.pushKeyEvent(&event, true); break; }
				case SDL_MOUSEBUTTONDOWN: { input.pushMouseButtonEvent(&event, false); break; }
				case SDL_MOUSEBUTTONUP: { input.pushMouseButtonEvent(&event, true); break; }
				case SDL_MOUSEWHEEL: { input.pushMouseWheelEvent(&event); break; }
				case SDL_MOUSEMOTION: { input.pushMouseMovementEvent(&event); break; }
				default: { break; }
				}
				onEvent(&event);
			}
			surface = SDL_GetWindowSurface(window);
			if (tex != nullptr) {
				SDL_DestroyTexture(tex);
			}
			format = surface->format;
			tex = SDL_CreateTextureFromSurface(renderer, surface);
			if (tex == nullptr) { return -2; }
			onLoop();
			onRender();
		}
		onExit();
		postExit();
		return 0;
	}

	void setPixel(SDL_Surface* s, int x, int y, uint32_t pixel) {
		if (tex == nullptr) { return; }
		if (x < 0 || x >= s->w || y < 0 || y >= s->h) { return; }
		((uint32_t*)s->pixels)[y * s->w + x] = pixel;
	}

private:
	virtual bool programInit() = 0;
	virtual void onEvent(SDL_Event* event) = 0;
	virtual void onLoop() = 0;
	virtual void onExit() = 0;

	const bool init(const char* title, int width, int height) {
		this->height = height;
		this->width = width;

		if (SDL_Init(SDL_INIT_VIDEO) < 0) { return false; } // initialise SDL2

		// create the window and renderer
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
		if (window == NULL) { return false; }
		renderer = SDL_CreateRenderer(window, -1, 0);

		// initialise the image instance.
		surface = SDL_GetWindowSurface(window);
		format = surface->format;
		tex = SDL_CreateTextureFromSurface(renderer, surface);
		if (tex == nullptr) { return false; }

		// Create some colour variations (uv map) for a test screen
		for (int x = 0; x < width; x++) { for (int y = 0; y < height; y++) { setPixel(surface, x, y, SDL_MapRGB(format, static_cast<uint8_t>((static_cast<float>(x) / static_cast<float>(width)) * 255.0f), static_cast<uint8_t>((static_cast<float>(y) / static_cast<float>(height)) * 255.0f), 0)); } }
		onRender(); // push test image to screen to show that program has started
		return programInit(); // initialise the program
	};

	// function that gets called after the programs loop function to display the image buffer
	void onRender() {
		if (tex != nullptr) {
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // set the background colour to black
			SDL_RenderClear(renderer);  // clear the display buffer
			SDL_RenderCopy(renderer, tex, NULL, NULL);
			SDL_RenderPresent(renderer); // show the display buffer
			SDL_DestroyTexture(tex);
		}
	};

	void postExit() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		window = nullptr;
		SDL_Quit();
	}
};