/*
  ==============================================================================

    WavetablePlot.cpp
    Created: 31 Dec 2022 2:47:51pm
    Author:  Arthur

  ==============================================================================
*/

#include "WavetablePlot.h"
#include <thread>
#include <cmath>

#define rdi(n) (int)round(n)


#if SDL_ACTIVE

WavetablePlot::WavetablePlot()
{
	init = false;
	quit = true;
}

WavetablePlot::~WavetablePlot()
{
	stop();
}

void WavetablePlot::setup(int width, int height)
{
	if (!quit) return;
	this->width = width;
	this->height = height;
	init = true;
	quit = windowQuit = false;
}

void WavetablePlot::setDrawData(list<std::complex<float>> l, std::function<float(std::complex<float>)> f)
{
	std::unique_lock lock(dataCopy);
	newData = l;
	convert = f;
	isNewData = true;
}

void WavetablePlot::drawLoop()
{
	// SDL boilerplate
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Wavetable Plot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_HIDDEN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT)
				quit = windowQuit = true;

		if (isNewData)
		{
			// copy vector
			{
				std::unique_lock lock(dataCopy);
				data = newData;
				isNewData = false;
			}

			if (data.empty())
				continue;
			else
				SDL_ShowWindow(window);

			list<float> v = data.mapTo(convert);

			// scale? todo continous rescaling
			if (yView == 0)
			{
				float min = v.reduce([](float a, float b) { return std::min(a, b); });
				float max = v.reduce([](float a, float b) { return std::max(a, b); });
				float p = std::max(std::abs(min), std::abs(max));
				yView = 2.1 * p;
			}

			// draw
			double vx = (double)width / (v.length()+2);
			double vy = (double)height / yView;
			double xAxis = (double)height / 2;

			// always keep two adjacent function values
			double y0 = xAxis - v[0], y1;

			// clear background
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(renderer);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

			for (int x = 1; x < v.length(); x++)
			{
				y1 = xAxis - v[x] * vy;

				// draw line
				SDL_RenderDrawLine(renderer, rdi((x + 0.5) * vx), rdi(y0), rdi((x + 1.5) * vx), rdi(y1));

				y0 = y1;
			}

			// update screen
			SDL_RenderPresent(renderer);
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void WavetablePlot::start()
{
	if (!init) return;
	drawThread = new std::thread(&WavetablePlot::drawLoop, this);
}

void WavetablePlot::stop()
{
	if (!init) return;
	quit = true;
	drawThread->join();
	delete drawThread;
	drawThread = nullptr;
}

#else

WavetablePlot::WavetablePlot() {}
WavetablePlot::~WavetablePlot() {}
void WavetablePlot::setup(int width, int height) {}
void WavetablePlot::setDrawData(list<std::complex<float>> l, std::function<float(std::complex<float>)> f) {}
void WavetablePlot::drawLoop() {}
void WavetablePlot::start() {}
void WavetablePlot::stop() {}

#endif  // SDL_ACTIVE
