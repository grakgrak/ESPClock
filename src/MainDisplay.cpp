#include "MainDisplay.h"

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <Ticker.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//------------------------------------------------------------------------
extern TFT_eSPI tft;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

String lastTimeShown = "88 88";
Ticker clockTimer;
Ticker weatherTimer;

//------------------------------------------------------------------------
void MainDisplay::setup()
{
    // setup the timers
    clockTimer.attach(0.5, CheckTime);
    weatherTimer.attach(300, CheckWeather); // every 5 mins

    BlankDisplay();

    CheckWeather(); // update the weather info
}

//------------------------------------------------------------------------
void MainDisplay::BlankDisplay()
{
    lastTimeShown = "88 88"; // so that the clock is re-rendered
    tft.fillScreen(TFT_BLACK);
}
//------------------------------------------------------------------------
void MainDisplay::CheckTime()
{
    static bool flash = false;

    if(WiFi.isConnected() == false)
        return;

    timeClient.update();

    String hours(timeClient.getHours());
    String mins(timeClient.getMinutes());
    if (hours.length() == 1)
        hours = "0" + hours;
    if (mins.length() == 1)
        mins = "0" + mins;

    //String tm = hours + (flash ? ":" :  " ") + mins;
    String tm = hours + " " + mins;
    if (tm != lastTimeShown)
    {
        lastTimeShown = tm;

        tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
        tft.setTextSize(2);
        tft.drawString(tm, 0, 0, 6);
    }

    flash = (flash == false);
    tft.fillCircle(120, 20, 3, flash ? 0xFBE0 : TFT_BLACK);
    tft.fillCircle(120, 50, 3, flash ? 0xFBE0 : TFT_BLACK);

    tft.setTextSize(1); // reset the text size
}

//------------------------------------------------------------------------
String MainDisplay::getMeteoconIcon(const String &icon)
{
    static String icons[18] = {"01d", "01n", "02d", "02n", "03d", "03n", "04d", "04n", "09d", "09n", "10d", "10n", "11d", "11n", "13d", "13n", "50d", "50n" };
    static String chars[18] = {"B", "C", "H", "4", "N", "5", "Y", "%", "R", "8", "Q", "7", "P", "6", "W", "#", "M", "M" };

    for(int i = 0; i < 18; ++i )
    if( icon == icons[i])
        return chars[i];

    // Nothing matched: N/A
    return ")";
}
//------------------------------------------------------------------------
#define CLOCK_HEIGHT 96
#define LINE_HEIGHT 28
void MainDisplay::CheckWeather()
{
    if(WiFi.isConnected() == false)
        return;

    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?id=2646504&APPID=312451d256875d1df6115712df54037b";

    String payload = "";

    http.setReuse(false);
    http.setTimeout(10000);
    http.begin(url);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode <= 0)
    {
        Serial.println("[HTTP] GET error code: " + http.errorToString(httpCode));
        http.end();
        return;
    }

    // file found at server
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.println("[HTTP] GET error code: " + http.errorToString(httpCode));
        http.end();
        return;
    }

    payload = http.getString();
    Serial.println("[HTTP] Payload: " + payload);
    http.end();

    /** Buffer for incoming JSON string */
    const size_t capacity = JSON_ARRAY_SIZE(3) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 340;
    DynamicJsonDocument jsonDoc(capacity);

    DeserializationError error = deserializeJson(jsonDoc, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    // update display
    BlankDisplay();
    CheckTime(); // re-display the clock

    JsonObject main = jsonDoc["main"];
    JsonObject weather = jsonDoc["weather"][0];
    JsonObject clouds = jsonDoc["clouds"];

    double temperature = main["temp"];
    String tmp(temperature - 273.15); // convert to celsius

    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setTextDatum(TR_DATUM); // top right
    tft.drawString("Temp:", 115, CLOCK_HEIGHT, 4);
    tft.drawString("Pressure:", 115, CLOCK_HEIGHT + (LINE_HEIGHT * 1), 4);
    tft.drawString("Humidity:", 115, CLOCK_HEIGHT + (LINE_HEIGHT * 2), 4);

    tft.setTextDatum(TL_DATUM); // top left
    tft.drawString(tmp, 120, CLOCK_HEIGHT, 4);
    tft.setFreeFont(&Meteocons_Regular_26);
    tft.drawString("*", 180, CLOCK_HEIGHT - 4, 1);

    tft.drawString( main["pressure"].as<String>() + " hPa", 120, CLOCK_HEIGHT + (LINE_HEIGHT * 1), 4);
    tft.drawString( main["humidity"].as<String>() + "%", 120, CLOCK_HEIGHT + (LINE_HEIGHT * 2), 4);


    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    tmp = weather["main"].as<String>() + " " + clouds["all"].as<String>() + "%";
    tft.drawString(tmp, 6, CLOCK_HEIGHT + (LINE_HEIGHT * 3), 4);
    tmp = "(" + weather["description"].as<String>() + ")";
    tft.drawString(tmp, 6, CLOCK_HEIGHT + (LINE_HEIGHT * 4), 4);

    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Note: the new fonts do not draw the background colour
    tft.setTextSize(2);
    tft.setFreeFont(&Meteocons_Regular_26);
    tft.drawString(getMeteoconIcon(weather["icon"].as<String>()), 180, 180, 1);
    tft.setTextSize(1); // reset the text size
}

