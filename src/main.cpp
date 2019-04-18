#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "..\..\Credentials.h"  // contains definitions of WIFI SSID and password

#define BUTTON_LEFT_PIN    16
#define BUTTON_RIGHT_PIN    17
#define LDR_PIN         34

#define BACKLIGHT_PIN   26
#define BACKLIGHT_CHANNEL   0
#define TONE_PIN        33
#define TONE_CHANNEL    1


const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

//------------------------------------------------------------------------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h

String lastTimeShown = "88 88";

Ticker clockTimer;
Ticker ldrTimer;
Ticker weatherTimer;
//------------------------------------------------------------------------
void BlankDisplay()
{
    lastTimeShown = "88 88";    // so that the clock is re-rendered
    tft.fillScreen(TFT_BLACK);
}
//------------------------------------------------------------------------
void CheckTime()
{
    static bool flash = false;

    timeClient.update();

    String hours( timeClient.getHours());
    String mins( timeClient.getMinutes());
    if( hours.length() == 1)
        hours = "0" + hours;
    if( mins.length() == 1)
        mins = "0" + mins;

    //String tm = hours + (flash ? ":" :  " ") + mins;
    String tm = hours + " " + mins;
    if( tm != lastTimeShown)
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
void CheckLDR()
{
    uint16_t ldr = analogRead(LDR_PIN);

    // adjust the brightness of the backlight
    int val = 254.0 / 4095.0 * (4095 - ldr);
    ledcWrite(BACKLIGHT_CHANNEL, 1 + val);
}

//------------------------------------------------------------------------
void CheckWeather()
{
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
    const size_t capacity = JSON_ARRAY_SIZE(3) + 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 340;
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
    CheckTime();    // re-display the clock

    JsonObject main = jsonDoc["main"];   
    JsonObject weather = jsonDoc["weather"][0];

    double temp = main["temp"];
    String tmp(temp - 273.15);  // convert to celsius

    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.drawString("Temp: " + tmp, 6, 100, 4);
    tft.drawString("Pressure: " + main["pressure"].as<String>(), 6, 130, 4);
    tft.drawString("Humidity: " + main["humidity"].as<String>(), 6, 160, 4);

    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);

    tmp = weather["main"].as<String>() + " (" + weather["description"].as<String>() + ")";
    tft.drawString(tmp, 6, 190, 4);


    // {"coord":{"lon":-0.17,"lat":50.83},
    // "weather":[
    //     {"id":801,"main":"Clouds","description":"few clouds","icon":"02d"}
    //  ],
    // "base":"stations",
    // "main":{"temp":282.14,"pressure":1025,"humidity":57,"temp_min":280.93,"temp_max":283.15},
    // "visibility":10000,
    // "wind":{"speed":5.7,"deg":130},
    // "clouds":{"all":20},
    // "dt":1555251484,
    // "sys":{"type":1,"id":1395,"message":0.0048,"country":"GB","sunrise":1555218537,"sunset":1555267962},
    // "id":2646504,
    // "name":"Hove",
    // "cod":200}
}

//------------------------------------------------------------------------
void IRAM_ATTR left_button()
{
    if(digitalRead(BUTTON_LEFT_PIN))
        tft.drawCircle(235,230,5, TFT_WHITE);
    else
        tft.drawCircle(235,230,5, TFT_BLACK);
}
//------------------------------------------------------------------------
void IRAM_ATTR right_button()
{
    if(digitalRead(BUTTON_RIGHT_PIN))
        tft.drawCircle(230,230,5, TFT_WHITE);
    else
        tft.drawCircle(230,230,5, TFT_BLACK);
}
//------------------------------------------------------------------------
void setup(void)
{
    Serial.begin(115200);

    // set pins
    pinMode(LDR_PIN, INPUT);
    pinMode(BUTTON_RIGHT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT_PIN), right_button, CHANGE);
    pinMode(BUTTON_LEFT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT_PIN), left_button, CHANGE);

    // init the TFT Display
    tft.init();
    tft.setRotation(3);
    BlankDisplay();

    tft.setTextColor(TFT_BLUE, TFT_BLACK); // Note: the new fonts do not draw the background colour
    tft.drawString("WiFi...", 10, 50, 4);

    // start the WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("ESPClock");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());

    BlankDisplay();

    // init PWM channels
    ledcSetup(BACKLIGHT_CHANNEL, 8000, 8);
    ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
    ledcWrite(BACKLIGHT_CHANNEL, 255);

    // ledcSetup(TONE_CHANNEL, 2700, 8);
    // ledcAttachPin(TONE_PIN, TONE_CHANNEL);

    // ledcWriteTone(TONE_CHANNEL, 2700);
    // delay(100);
    // ledcWriteTone(TONE_CHANNEL, 0);

    // setup the timers
    clockTimer.attach(0.5, CheckTime);
    ldrTimer.attach(1, CheckLDR);
    weatherTimer.attach(300, CheckWeather); // every 5 mins

    CheckWeather(); // update the weather info
}

//------------------------------------------------------------------------
void loop()
{
    delay(500);
}
