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
#include <chrono>

#define rdi(n) (int)round(n)
#define NANOS_5S	5000000000
#define NANOS_1S	1000000000
#define NANOS_500MS  500000000
#define NANOS_250MS  250000000


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

void WavetablePlot::setDrawData(list<std::complex<float>> l, std::function<float(std::complex<float>)> f, list<float> l2)
{
	std::unique_lock lock(dataCopy);
	newData = l;
	newData2 = l2;
	convert = f;
	isNewData = true;
}

inline long long WavetablePlot::getTimestampNow()
{
	return std::chrono::steady_clock::now().time_since_epoch().count();
}


void WavetablePlot::drawLoop()
{
	// SDL boilerplate
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Wavetable Plot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_HIDDEN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	
	// scaling
	const float oversize = 2.5f;
	lastRescale = getTimestampNow();

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
				data2 = newData2;
				isNewData = false;
			}

			if (data.empty())
				continue;
			else
				SDL_ShowWindow(window);

			list<float> v = data.mapTo(convert);

			// scale?
			const auto now = getTimestampNow();

			//if (yView == 0 || now - lastRescale >= NANOS_250MS)
			{
				float min = v.reduce([](float a, float b) { return std::min(a, b); });
				float max = v.reduce([](float a, float b) { return std::max(a, b); });
				float p = std::max(std::abs(min), std::abs(max));
				
				float r = (p * oversize - yView) / yView;
				if (yView == 0											// init view
					|| r > 0.1f											// expand view
					/*|| r < -0.2f && now - lastRescale >= NANOS_5S*/)		// shrink view (produces confusing visuals)
				{
					yView = p * oversize;
					lastRescale = now;
				}
			}

			// draw
			double vx = (double)width / (v.length()+2);
			double vy = (double)height / yView;
			double xAxis = (double)height / 2;

			// always keep two adjacent function values
			double y0, y1;

			// clear background
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(renderer);

			// data2
			SDL_SetRenderDrawColor(renderer, 170, 0, 220, SDL_ALPHA_OPAQUE);
			y0 = xAxis - data2[0] * vy;

			for (int x = 0; x < v.length(); x++)
			{
				y1 = xAxis - data2[x] * vy;
				// draw line
				SDL_RenderDrawLine(renderer, rdi((x+1 - 0.5) * vx), rdi(y0), rdi((x+1 + 0.5) * vx), rdi(y1));
				y0 = y1;
			}

			// data
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
			y0 = xAxis - v[0] * vy;

			for (int x = 1; x < v.length(); x++)
			{
				y1 = xAxis - v[x] * vy;
				// draw line
				SDL_RenderDrawLine(renderer, rdi((x + 1 - 0.5) * vx), rdi(y0), rdi((x + 1 + 0.5) * vx), rdi(y1));
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
void WavetablePlot::setDrawData(list<std::complex<float>> l, std::function<float(std::complex<float>)> f, list<float> l2) {}
void WavetablePlot::drawLoop() {}
void WavetablePlot::start() {}
void WavetablePlot::stop() {}

#endif  // SDL_ACTIVE
