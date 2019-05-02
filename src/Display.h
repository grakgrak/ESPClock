#pragma once
#include "Button.h"

enum TScreenName
{
    NONE,
    ALARM_SCREEN,
    WEATHER_SCREEN,
    KEYBOARD_SCREEN
};

namespace Display
{
void BlankArea(int top, int bottom);
void HandleKeys(TButtonEvent left, TButtonEvent right);
void setup();
} // namespace Display