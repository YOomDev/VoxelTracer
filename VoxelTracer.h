#pragma once

#include <algorithm>
#include <execution>
#include "SDLWindowEngine.h"
#include "Vec3.h"
#include "World.h"
#include "Screenshot.h"
#include "Camera.h"

constexpr int SC_WIDTH = 1920;
constexpr int SC_HEIGHT = 1080;

struct Engine : public SDLWindowEngine {
private:
	World world;
	Camera cam;

	virtual bool programInit() override {

		// TODO: init cam



		return true;
	};

	vec3 traceRay(Ray ray, int bounces, float& depth) const {
		depth = 2;
	}

	void renderToSurface(SDL_Surface* s) {
		cam.prepare({ 0,0,0 });

		std::vector<uint32_t> vertIter, horIter;
		vertIter.clear();
		vertIter.resize(s->h);
		for (int i = 0; i < s->h; i++) { vertIter[i] = i; }
		horIter.clear();
		horIter.resize(s->w);
		for (int i = 0; i < s->w; i++) { horIter[i] = i; }

		std::for_each(std::execution::par, vertIter.begin(), vertIter.end(), [this, s, horIter](uint32_t y) {
			std::for_each(std::execution::par, horIter.begin(), horIter.end(), [this, s, y](uint32_t x) {
				vec3 color = { 0,0,0 };
				Ray ray = cam.get_ray(static_cast<float>(x) / static_cast<float>(s->w), static_cast<float>(y) / static_cast<float>(s->h));

				// TODO: trace ray

				setPixel(s, x, y, SDL_MapRGBA(format, ((uint8_t)color.r() * 255.99f), ((uint8_t)color.g() * 255.99f), ((uint8_t)color.b() * 255.99f), 255));
			});
		});
	}

	virtual void onEvent(SDL_Event* event) override {
		switch (event->type) {
		case SDL_EventType::SDL_APP_TERMINATING:
		case SDL_EventType::SDL_QUIT:
			isRunning = false;
			break;
		default:
			break;
		}
	};

	virtual void onLoop() override {
		world.loadChunks();

		// TODO: use input to move camera
		float depth = 0;
		cam.prepare({ 0,0,0 });
		traceRay(cam.get_ray(0.5f, 0.5f), 0, depth);

		uint64_t start = getTime();
		renderToSurface(surface);
		uint64_t us = getTime() - start;
		printf_s("Rendering the frame took %d us (%d ms)\n", us, us / 1000);

		if (input.isKeyPressed(SDLK_q)) {
			SDL_Surface* screenshot = SDL_CreateRGBSurfaceWithFormat(NULL, SC_WIDTH, SC_HEIGHT, 8, format->format);
			if (screenshot != nullptr) {
				start = getTime();
				renderToSurface(screenshot);
				us = getTime() - start;
				printf_s("Rendering the screenshot took %d us (%d ms)\n", us, us / 1000);
				save_surface_as_bmp(screenshot, "test.bmp");
			}
		}
	};

	virtual void onExit() override {};
};