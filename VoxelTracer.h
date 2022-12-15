#pragma once

#include <algorithm>
#include <execution>
#include "SDLWindowEngine.h"
#include "Screenshot.h"
#include "Tracing.h"

constexpr int SC_WIDTH = 1920;
constexpr int SC_HEIGHT = 1080;

struct Engine : public SDLWindowEngine {
private:
	World world;
	Camera cam;

	virtual bool programInit() override {
		// init materials
		world.materials.push_back(Material()); // air
		world.materials.push_back(Material(SOLID, {0.3f, 0.5f, 0.8f}, 0.3f, 0.3f)); // solid
		world.materials.push_back(Material(SOLID, {0.8f, 0.3f, 0.5f}, 0.3f, 0.3f)); // reflective
		world.materials.push_back(Material(SOLID, {0.5f, 0.8f, 0.3f}, 0.3f, 0.3f)); // refractive

		cam = Camera({ 0, 1, 0 }, 50.0f, static_cast<float>(surface->w) / static_cast<float>(surface->h), 0.1f, 10.0f);

		return true;
	};

	void renderToSurface(SDL_Surface* s, const vec3& dir, int samples) {
		cam.prepare(dir);

		std::vector<uint32_t> vertIter, horIter;
		vertIter.clear();
		vertIter.resize(s->h);
		for (int i = 0; i < s->h; i++) { vertIter[i] = i; }
		horIter.clear();
		horIter.resize(s->w);
		for (int i = 0; i < s->w; i++) { horIter[i] = i; }
		float wp = 1.0f / static_cast<float>(s->w), hp = 1.0f / static_cast<float>(s->h);

		{// Clear the screen
			int size = s->w * s->h;
			uint32_t* pixels = (uint32_t*)s->pixels;
			uint32_t t = SDL_MapRGBA(format, 0, 0, 0, 255);
			for (int i = 0; i < size; i++) {
				pixels[i] = t;
			}
		}

		if (samples > 1) {
			std::for_each(std::execution::par, vertIter.begin(), vertIter.end(), [this, s, wp, hp, horIter](uint32_t y) {
				std::for_each(std::execution::par, horIter.begin(), horIter.end(), [this, s, wp, hp, y](uint32_t x) {
					Ray ray = cam.get_ray(static_cast<float>(x) * wp, static_cast<float>(y) * hp);


					float depth = 0; // unused but required for 
					vec3 color = trace(cam.position, ray, world, 0, 1, depth);
					// TODO: maybe multisampling a pixel?


					setPixel(s, x, y, SDL_MapRGBA(format, ((uint8_t)color.r() * 255.99f), ((uint8_t)color.g() * 255.99f), ((uint8_t)color.b() * 255.99f), 255));
				});
			});
		}
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
		// Load the chunks requested in the last frame
		world.loadChunks();

		// TODO: use input to move camera

		// Prepare camera for rendering
		float depth = 0;
		trace(cam.position, cam.get_ray(0.5f, 0.5f), world, 0, 5, depth);
		cam.focusDistance = depth;

		// Render image
		uint64_t start = getTime();
		renderToSurface(surface, { 3,5,8 }, 1);
		uint64_t us = getTime() - start;
		printf_s("Rendering the frame took %d us (%d ms)\n", us, us / 1000);

		// Render screenshot if needed
		if (input.isKeyPressed(SDLK_q)) {
			SDL_Surface* screenshot = SDL_CreateRGBSurfaceWithFormat(NULL, SC_WIDTH, SC_HEIGHT, 8, format->format);
			if (screenshot != nullptr) {
				start = getTime();
				renderToSurface(screenshot, {3,5,8}, 10);
				us = getTime() - start;
				printf_s("Rendering the screenshot took %d us (%d ms)\n", us, us / 1000);
				save_surface_as_bmp(screenshot, "test.bmp");
			}
		}
	};

	virtual void onExit() override {};
};