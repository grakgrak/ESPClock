#pragma once

class WeatherDisplay
{
private:
    static void FetchWeather();
    static void DrawWeather();
public:
    static void setup();
    static void start();
    static void stop();
};