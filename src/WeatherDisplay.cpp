#include "Shared.h"
#include "WeatherDisplay.h"

#define TOP CLOCK_HEIGHT

//------------------------------------------------------------------------
String _payload;
Ticker weatherTimer;
Ticker showWeatherTimer;
//------------------------------------------------------------------------
void WeatherDisplay::start()
{
    logln("start WeatherDisplay");

    if( _payload.length() == 0)
        FetchWeather();

    // setup the timers
    showWeatherTimer.attach(60, DrawWeather);
    
    DrawWeather();
}
//------------------------------------------------------------------------
void WeatherDisplay::stop()
{
    logln("stop WeatherDisplay");
    showWeatherTimer.detach();
}
//------------------------------------------------------------------------
void WeatherDisplay::setup()
{
    weatherTimer.attach(300, FetchWeather); // every 5 mins
}

//------------------------------------------------------------------------
void WeatherDisplay::FetchWeather()
{
    if(WiFi.isConnected() == false)
        return;

    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?id=2646504&APPID=312451d256875d1df6115712df54037b";

    http.setReuse(false);
    http.setTimeout(10000);
    http.begin(url);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode <= 0)
    {
        logln("[HTTP] GET error code: " + http.errorToString(httpCode));
        http.end();
        return;
    }

    // file found at server
    if (httpCode != HTTP_CODE_OK)
    {
        logln("[HTTP] GET error code: " + http.errorToString(httpCode));
        http.end();
        return;
    }

    _payload = http.getString();
    logln("[HTTP] Payload: " + _payload);
    http.end();
}

//------------------------------------------------------------------------
String getMeteoconIcon(const String &icon)
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
void WeatherDisplay::DrawWeather()
{
    if( _payload.length() == 0)
    {
        tft.setTextSize(1);
        tft.drawString("Waiting for data...", 0, TOP, 4);
        return;
    }

    /** Buffer for incoming JSON string */
    const size_t capacity = JSON_ARRAY_SIZE(3) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 340;
    DynamicJsonDocument jsonDoc(capacity);

    DeserializationError error = deserializeJson(jsonDoc, _payload);
    if (error)
    {
        log(F("deserializeJson() failed: "));
        logln(error.c_str());
        return;
    }

    // update display
    JsonObject main = jsonDoc["main"];
    JsonObject weather = jsonDoc["weather"][0];
    JsonObject clouds = jsonDoc["clouds"];

    double temp = main["temp"];
    gTemperature = String(temp - 273.15); // convert to celsius

    // blank the display area
    TDisplay::BlankArea(TOP, TFT_HEIGHT);
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setTextDatum(TR_DATUM); // top right
    tft.drawString("Temp:", 115, TOP, 4);
    tft.drawString("Pressure:", 115, TOP + (LINE_HEIGHT * 1), 4);
    tft.drawString("Humidity:", 115, TOP + (LINE_HEIGHT * 2), 4);

    tft.setTextDatum(TL_DATUM); // top left
    tft.drawString(gTemperature, 120, TOP, 4);
    tft.setFreeFont(&Meteocons_Regular_26);
    tft.drawString("*", 185, TOP - 4, 1);

    gPressure = main["pressure"].as<String>();
    gHumidity = main["humidity"].as<String>();

    tft.drawString( gPressure + " hPa", 120, TOP + (LINE_HEIGHT * 1), 4);
    tft.drawString( gHumidity + "%", 120, TOP + (LINE_HEIGHT * 2), 4);

    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    String tmp = weather["main"].as<String>() + " " + clouds["all"].as<String>() + "%";
    tft.drawString(tmp, 6, TOP + (LINE_HEIGHT * 3), 4);
    tmp = "(" + weather["description"].as<String>() + ")";
    tft.drawString(tmp, 6, TOP + (LINE_HEIGHT * 4), 4);

    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Note: the new fonts do not draw the background colour
    tft.setTextSize(2);
    tft.setFreeFont(&Meteocons_Regular_26);
    tft.drawString(getMeteoconIcon(weather["icon"].as<String>()), 180, 180, 1);
}
