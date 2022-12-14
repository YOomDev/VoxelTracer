#pragma once

#include <algorithm>
#include <execution>
#include "SDLWindowEngine.h"
#include "Vec3.h"

struct Engine : public SDLWindowEngine {
	std::vector<uint32_t> vertIter, horIter;
	int w, h;

	void resize() {
		w = surface->w;
		h = surface->h;
		vertIter.clear();
		vertIter.resize(h);
		for (int i = 0; i < h; i++) { vertIter[i] = i; }
		horIter.clear();
		horIter.resize(w);
		for (int i = 0; i < w; i++) { horIter[i] = i; }
	}

	virtual bool programInit() override {
		resize();
		
		// TODO: init cam



		return true;
	};

	virtual void onEvent(SDL_Event* event) override {
		if (event->type == SDL_EventType::SDL_APP_TERMINATING || event->type == SDL_EventType::SDL_QUIT) { isRunning = false; }
	};

	virtual void onLoop() override {
		if (surface->w != w || surface->h != h) {
			resize();
		}

		// TODO: use input to move camera

		uint64_t start = getTime();

		std::for_each(std::execution::par, vertIter.begin(), vertIter.end(), [this](uint32_t y) { 
			std::for_each(std::execution::par, horIter.begin(), horIter.end(), [this, y](uint32_t x) {
				vec3 color = { 0,0,0 };

				// TODO ray trace

				setPixel(x, y, SDL_MapRGBA(format, ((uint8_t)color.r() * 255.99f), ((uint8_t)color.g() * 255.99f), ((uint8_t)color.b() * 255.99f), 255));
			});
		});

		// tmp
		int x = surface->w / 2;
		int y = surface->h / 2;
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				setPixel(x - i, y - j, SDL_MapRGBA(format, (uint8_t)255, (uint8_t)255, (uint8_t)255, (uint8_t)255));
			}
		}

		uint64_t us = getTime() - start;
		printf_s("rendering took %d us (%d ms)\n", us, us / 1000);
	};

	virtual void onExit() override {};
};