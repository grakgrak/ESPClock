#include "Shared.h"
#include "Display.h"
#include "WeatherDisplay.h"
#include "Clock.h"
#include "Comms.h"

namespace WeatherDisplay
{
//------------------------------------------------------------------------
Ticker showWeatherTimer;

int PageIndex = 0;
int DataIndex = 0;
struct WeatherData
{
    String Temp;
    String Humidity;
    String Pressure;
    String Condition;
    String Icon;
    String Detail;
    String Clouds;
    String Sunrise;
    String Sunset;
    String MinTemp;
    String MaxTemp;
} Data[1];

//------------------------------------------------------------------------
String getFormattedTime(unsigned long unixtime)
{
    unixtime += Clock::dstOffset(); // get the daylight savings time offset seconds....

    String hours((unixtime % 86400L) / 3600);
    if( hours.length() == 1)
        hours = "0" + hours;

    String minutes((unixtime % 3600) / 60);
    if( minutes.length() == 1)
        minutes = "0" + minutes;

    String seconds(unixtime % 60);
    if( seconds.length() == 1)
        seconds = "0" + seconds;

    return hours + ":" + minutes + ":" + seconds;
}

//------------------------------------------------------------------------
void AddMqttSubscribeTopics()
{
    Comms::SubscribeTopic("Weather/Today/#"); // get the weather data for today
}

//------------------------------------------------------------------------
void HandleMqttMessage(const char *topic, const char *payload)
{
    int idx = -1; 
    String topicKey;

    // check for weather related messages
    if (strncmp("Weather/Today/", topic, 14) == 0)
    {
        topicKey = String(topic + 14);
        idx = 0;
    }

    if( idx >= 0 && topicKey != "")
    {
        WeatherData &data = Data[idx];

        if (topicKey =="Temp")
            data.Temp = payload;
        else if (topicKey =="Humidity")
            data.Humidity = payload;
        else if (topicKey =="Pressure")
            data.Pressure = payload;
        else if (topicKey =="Condition")
            data.Condition = payload;
        else if (topicKey =="Icon")
            data.Icon = payload;
        else if (topicKey =="Detail")
            data.Detail = payload;
        else if (topicKey =="Clouds")
            data.Clouds = payload;
        else if (topicKey =="Sunrise")
            data.Sunrise = payload;
        else if (topicKey =="Sunset")
            data.Sunset = payload;
        else if (topicKey =="MinTemp")
            data.MinTemp = payload;
        else if (topicKey =="MaxTemp")
            data.MaxTemp = payload;
    }
}

//------------------------------------------------------------------------
String getMeteoconIcon(const String &icon)
{
    static String icons[18] = {"01d", "01n", "02d", "02n", "03d", "03n", "04d", "04n", "09d", "09n", "10d", "10n", "11d", "11n", "13d", "13n", "50d", "50n"};
    static String chars[18] = {"B", "C", "H", "4", "N", "5", "Y", "%", "R", "8", "Q", "7", "P", "6", "W", "#", "M", "M"};

    for (int i = 0; i < 18; ++i)
        if (icon == icons[i])
            return chars[i];

    // Nothing matched: N/A
    return ")";
}
//------------------------------------------------------------------------
void DrawTagValue(TFT_eSPI &tft, const String &tag, const String &value, int line, const String &symbol)
{
    int y = CLOCK_HEIGHT + (LINE_HEIGHT * line);
    tft.setTextSize(1);
    tft.setTextDatum(TR_DATUM); // top right
    tft.drawString(tag, 115, y, 4);

    tft.setTextDatum(TL_DATUM); // top left
    tft.drawString(value, 120, y, 4);
    if (symbol != "")
    {
        int w = tft.textWidth(value,4);
        tft.setFreeFont(&Meteocons_Regular_26);
        tft.drawString("*", 120 + w, y - 4, 1); // deg C icon
    }
}
//------------------------------------------------------------------------
void DrawLine(TFT_eSPI &tft, const String &text, int line)
{
    int y = CLOCK_HEIGHT + (LINE_HEIGHT * line);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM); // top left
    tft.drawString(text, 6, y, 4);
}
//------------------------------------------------------------------------
void DrawWeather()
{
    auto tft = Display::Lock();

    // blank the display area
    Display::BlankArea(CLOCK_HEIGHT, TFT_HEIGHT);

    WeatherData &data = Data[DataIndex];

    if (PageIndex == 0)
    {
        tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
        DrawTagValue(tft, "Temp:", data.Temp, 0, "*");
        DrawTagValue(tft, "Pressure:", data.Pressure + " hPa", 1, "");
        DrawTagValue(tft, "Humidity:", data.Humidity + "%", 2, "");

        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        DrawLine(tft, data.Condition + " " + data.Clouds + "%", 3);
        DrawLine(tft, "(" + data.Detail + ")", 4);

        tft.setTextColor(TFT_WHITE, TFT_BLACK); // Note: the new fonts do not draw the background colour
        tft.setTextSize(2);
        tft.setFreeFont(&Meteocons_Regular_26);
        tft.drawString(getMeteoconIcon(data.Icon), 180, 180, 1);
    }
    else
    {
        tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
        DrawTagValue(tft, "MinTemp:", data.MinTemp, 0, "*");
        DrawTagValue(tft, "MaxTemp:", data.MaxTemp, 1, "*");
        DrawTagValue(tft, "Sunrise:", getFormattedTime(atol(data.Sunrise.c_str())), 2, "");
        DrawTagValue(tft, "Sunset:", getFormattedTime(atol(data.Sunset.c_str())), 3, "");
    }
    Display::Release();
}
//------------------------------------------------------------------------
void HandleKeys(TButtonEvent left, TButtonEvent right)
{
    if (left == TButtonEvent::PRESS || right == TButtonEvent::PRESS)
    {
        PageIndex = (PageIndex + 1) % 2;
        DrawWeather();
    }
}

//------------------------------------------------------------------------
void start()
{
    logln("start WeatherDisplay");

    // setup the timers
    showWeatherTimer.attach(30, DrawWeather);

    DataIndex = 0;
    PageIndex = 0;
    DrawWeather();
}
//------------------------------------------------------------------------
void stop()
{
    logln("stop WeatherDisplay");
    showWeatherTimer.detach();
}

} // namespace WeatherDisplay