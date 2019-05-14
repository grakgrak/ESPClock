#include "Shared.h"
#include "Display.h"
#include "AlarmDisplay.h"
#include "ClockDisplay.h"
#include "KeyboardDisplay.h"
#include "WeatherDisplay.h"

namespace Display
{
#define BACKLIGHT_CHANNEL 0

enum TScreenName
{
    NONE,
    ALARM_SCREEN,
    WEATHER_SCREEN,
    KEYBOARD_SCREEN
};

//------------------------------------------------------------------------
TFT_eSPI _tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
SemaphoreHandle_t xSemaphore;

Ticker ldrTimer;
TScreenName currentScreen = TScreenName::NONE;

//------------------------------------------------------------------------
TFT_eSPI &Lock()
{
    xSemaphoreTakeRecursive(xSemaphore, 100);
    return _tft;
}
//------------------------------------------------------------------------
void Release()
{
    xSemaphoreGiveRecursive(xSemaphore);
}

//------------------------------------------------------------------------
void CheckLDR()
{
    uint16_t ldr = analogRead(LDR_PIN);

    // adjust the brightness of the backlight
    int val = 254.0 / 4095.0 * (4095 - ldr);
    ledcWrite(BACKLIGHT_CHANNEL, 1 + val);
}

//------------------------------------------------------------------------
void BlankArea(int top, int bottom)
{
    Lock().fillRect(0, top, TFT_WIDTH, bottom - top, TFT_BLACK);
    Release();
}

//------------------------------------------------------------------------
void ShowScreen(TScreenName screen)
{
    if (screen == currentScreen)
        return;

    switch (currentScreen)
    {
    case TScreenName::ALARM_SCREEN:
        AlarmDisplay::stop();
        break;
    case TScreenName::WEATHER_SCREEN:
        WeatherDisplay::stop();
        break;
    case TScreenName::KEYBOARD_SCREEN:
        KeyboardDisplay::stop();
        break;
    case TScreenName::NONE:
        break;
    default:
        break;
    }

    currentScreen = screen;

    switch (screen)
    {
    case TScreenName::ALARM_SCREEN:
        ClockDisplay::start(); // display the clock
        AlarmDisplay::start();
        break;
    case TScreenName::WEATHER_SCREEN:
        ClockDisplay::start(); // display the clock
        WeatherDisplay::start();
        break;
    case TScreenName::KEYBOARD_SCREEN:
        ClockDisplay::stop(); // Hide the clock
        KeyboardDisplay::start();
        break;
    case TScreenName::NONE:
        break;
    default:
        break;
    }
}
//------------------------------------------------------------------------
void ShowNextScreen()
{
    TScreenName next = TScreenName::WEATHER_SCREEN; // default to the weather screen
    switch (currentScreen)
    {
    case TScreenName::ALARM_SCREEN:
        next = TScreenName::WEATHER_SCREEN;
        break;
    case TScreenName::WEATHER_SCREEN:
        next = TScreenName::ALARM_SCREEN;
        break;
    case TScreenName::KEYBOARD_SCREEN:
        next = TScreenName::ALARM_SCREEN;
        break;
    default:
        break;
    }
    ShowScreen(next);
}
//------------------------------------------------------------------------
void ShowPriorScreen()
{
    TScreenName next = TScreenName::WEATHER_SCREEN; // default to the weather screen

    switch (currentScreen)
    {
    case TScreenName::ALARM_SCREEN:
        next = TScreenName::WEATHER_SCREEN;
        break;
    case TScreenName::WEATHER_SCREEN:
        next = TScreenName::ALARM_SCREEN;
        break;
    case TScreenName::KEYBOARD_SCREEN:
        next = TScreenName::WEATHER_SCREEN;
        break;
    default:
        break;
    }
    ShowScreen(next);
}
//------------------------------------------------------------------------
void HandleKeys(TButtonEvent left, TButtonEvent right)
{
    switch (currentScreen)
    {
    case TScreenName::ALARM_SCREEN:
        break;
    case TScreenName::WEATHER_SCREEN:
        WeatherDisplay::HandleKeys(left,right);
        break;
    case TScreenName::KEYBOARD_SCREEN:
        if (KeyboardDisplay::keypress(left, right))
            return;
        break;
    default:
        break;
    }

    if (left == TButtonEvent::LONG_PRESS && right == TButtonEvent::LONG_PRESS)
        ShowScreen(TScreenName::KEYBOARD_SCREEN);
    else
    {
        // switch screens on logn button press
        if (left == TButtonEvent::LONG_PRESS)
            ShowPriorScreen();

        if (right == TButtonEvent::LONG_PRESS)
            ShowNextScreen();
    }
}
//------------------------------------------------------------------------
void setup()
{
    xSemaphore = xSemaphoreCreateRecursiveMutex();
    // set pins
    pinMode(LDR_PIN, INPUT);

    _tft.init();
    _tft.setRotation(3);

    _tft.setTextColor(TFT_BLUE, TFT_BLACK); // Note: the new fonts do not draw the background colour
    _tft.setTextFont(4);
    _tft.print("WiFi");

    // init Backlight PWM
    ledcSetup(BACKLIGHT_CHANNEL, 4000, 8);
    ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
    ledcWrite(BACKLIGHT_CHANNEL, 255);

    // setup the display brightness
    ldrTimer.attach(1, CheckLDR);

    ShowScreen(TScreenName::WEATHER_SCREEN);
}
} // namespace Display