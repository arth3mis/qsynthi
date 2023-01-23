/*
  ==============================================================================

    WavetablePlot.h
    Created: 31 Dec 2022 2:47:51pm
    Author:  Arthur

  ==============================================================================
*/

#pragma once

#include "list.hpp"
#include <functional>
#include <complex>
#include <mutex>

#define SDL_ACTIVE __has_include("SDL.h") && false

#if SDL_ACTIVE
#include "SDL.h"
#endif

class WavetablePlot
{
private:
    list<std::complex<float>> data, newData;
    list<float> data2, newData2;
    std::function<float(std::complex<float>)> convert;
    bool isNewData;
    float yView = 0;

    long long lastRescale;
    inline long long getTimestampNow();

    std::thread *drawThread;
    std::mutex dataCopy;
    void drawLoop();

#if SDL_ACTIVE
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event e;
#endif
    int width;
    int height;
    bool init;
    bool quit, windowQuit;

public:
    WavetablePlot();
    ~WavetablePlot();
    void setup(int width, int height);
    void setDrawData(list<std::complex<float>> l, std::function<float(std::complex<float>)> f, list<float> l2 = {});
    void start();
    void stop();
    bool isQuit() { return quit; }
    bool isInnerQuit() { return windowQuit; }
};