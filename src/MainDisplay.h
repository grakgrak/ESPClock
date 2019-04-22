#pragma once

#include <ArduinoJson.h>

class MainDisplay
{
private:
    static void BlankDisplay();
    static void CheckWeather();
    static void CheckTime();
    static String getMeteoconIcon(const String &icon);

public:  
    static void setup();
};