#include "Shared.h"
#include "ClockDisplay.h"
#include "Clock.h"

namespace ClockDisplay
{

//------------------------------------------------------------------------
String lastTimeShown = "88 88";
Ticker clockTimer;

//------------------------------------------------------------------------
void DrawTime()
{
    static bool flash = false;

    if (WiFi.isConnected() == false)
        return;

    if( Clock::update() == false )
        return;

    String hours(Clock::getHours());
    String mins(Clock::getMinutes());
    if (hours.length() == 1)
        hours = "0" + hours;
    if (mins.length() == 1)
        mins = "0" + mins;

    auto tft = Display::Lock();

    String tm = hours + " " + mins;
    if (tm != lastTimeShown)
    {
        lastTimeShown = tm;

        tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
        tft.setTextSize(2);
        tft.drawString(tm, 0, 0, 6);
    }

    flash = (flash == false);
    tft.fillCircle(120, 20, 4, flash ? 0xFBE0 : TFT_BLACK);
    tft.fillCircle(120, 50, 4, flash ? 0xFBE0 : TFT_BLACK);

    Display::Release();
}
//------------------------------------------------------------------------
void start()
{
    logln("start ClockDisplay");
    // setup the timers
    clockTimer.attach(0.5, DrawTime);

    lastTimeShown = "88 88"; // so that the clock is re-rendered
    DrawTime();
}
//------------------------------------------------------------------------
void stop()
{
    logln("stop ClockDisplay");
    clockTimer.detach();
}

} // namespace ClockDisplay