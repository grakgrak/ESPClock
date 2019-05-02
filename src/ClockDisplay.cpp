#include "Shared.h"
#include "ClockDisplay.h"

namespace ClockDisplay
{
//------------------------------------------------------------------------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

String lastTimeShown = "88 88";
Ticker clockTimer;

//------------------------------------------------------------------------
void BlankDisplay()
{
    lastTimeShown = "88 88"; // so that the clock is re-rendered
    tft.fillScreen(TFT_BLACK);
}
//------------------------------------------------------------------------
void DrawTime()
{
    static bool flash = false;

    if (WiFi.isConnected() == false)
        return;

    timeClient.update();

    String hours(timeClient.getHours());
    String mins(timeClient.getMinutes());
    if (hours.length() == 1)
        hours = "0" + hours;
    if (mins.length() == 1)
        mins = "0" + mins;

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
}
//------------------------------------------------------------------------
int getHours()
{
    return timeClient.getHours();
}
//------------------------------------------------------------------------
int getMinutes()
{
    return timeClient.getMinutes();
}
//------------------------------------------------------------------------
int getDay()
{
    return timeClient.getDay();
}
//------------------------------------------------------------------------
void start()
{
    logln("start ClockDisplay");
    // setup the timers
    clockTimer.attach(0.5, DrawTime);

    BlankDisplay();
    DrawTime();
}
//------------------------------------------------------------------------
void stop()
{
    logln("stop ClockDisplay");
    clockTimer.detach();
}

} // namespace ClockDisplay