#pragma once
#include "Button.h"

enum TScreenName
{
    NONE,
    ALARM_SCREEN,
    WEATHER_SCREEN,
    KEYBOARD_SCREEN
};

class TDisplay
{
private:
    static void ShowScreen(TScreenName screen);
    static void ShowNextScreen();
    static void ShowPriorScreen();
public:
    static void BlankArea( int top, int bottom);

    static void HandleKeys(TButtonEvent left, TButtonEvent right);

    static void setup();
};