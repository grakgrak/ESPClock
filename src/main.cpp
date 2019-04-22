// todo: wifi setup
// location setting
//

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Ticker.h>
#include <WiFi.h>
#include <ArduinoOTA.h> // https://github.com/esp8266/Arduino/blob/master/libraries/ArduinoOTA/ArduinoOTA.h
#include <AsyncMqttClient.h>

#include "MainDisplay.h"
#include "Button.h"
#include "Alarm.h"
#include "..\..\Credentials.h" // contains definitions of WIFI SSID and password

#define MQTT_HOST IPAddress(192, 168, 1, 210)
#define MQTT_PORT 1883
#define PUBLISH_FREQ_SECS 10

#define BUTTON_LEFT_PIN 16
#define BUTTON_RIGHT_PIN 17
#define LDR_PIN 34

#define BACKLIGHT_PIN 26
#define BACKLIGHT_CHANNEL 0
#define TONE_PIN 33
#define TONE_CHANNEL 1
#define TONE_FREQ   2700

#define HOSTNAME "ESPClock"
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

TAlarm Alarms[10];

//------------------------------------------------------------------------
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
Ticker mqttPublishTimer;

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
Ticker ldrTimer;

// button handlers
TButton Right(BUTTON_RIGHT_PIN, []() -> void IRAM_ATTR { Right.HandleEvent(); });
TButton Left(BUTTON_LEFT_PIN, []() -> void IRAM_ATTR { Left.HandleEvent(); });

//------------------------------------------------------------------------
void Beep(int freq, int ms)
{
    pinMode(TONE_PIN, OUTPUT);
    ledcAttachPin(TONE_PIN, TONE_CHANNEL);

    ledcWriteTone(TONE_CHANNEL, freq);
    delay(ms);
    ledcWriteTone(TONE_CHANNEL, 0);

    ledcDetachPin(TONE_PIN);
    pinMode(TONE_PIN, INPUT);
}

//------------------------------------------------------------------------
void CheckLDR()
{
    uint16_t ldr = analogRead(LDR_PIN);

    // adjust the brightness of the backlight
    int val = 254.0 / 4095.0 * (4095 - ldr);
    ledcWrite(BACKLIGHT_CHANNEL, 1 + val);
}

//--------------------------------------------------------------------
void init_OTA()
{
    Serial.println("init OTA");

    // ArduinoOTA callback functions
    ArduinoOTA.onStart([]() {
        Serial.println("OTA starting...");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("OTA done.Reboot...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int prevPcnt = 100;
        unsigned int pcnt = (progress / (total / 100));
        unsigned int roundPcnt = 5 * (int)(pcnt / 5);
        if (roundPcnt != prevPcnt)
        {
            prevPcnt = roundPcnt;
            Serial.println("OTA upload " + String(roundPcnt) + "%");
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.print("OTA Error " + String(error) + ":");
        const char *line2 = "";
        switch (error)
        {
        case OTA_AUTH_ERROR:
            line2 = "Auth Failed";
            break;
        case OTA_BEGIN_ERROR:
            line2 = "Begin Failed";
            break;
        case OTA_CONNECT_ERROR:
            line2 = "Connect Failed";
            break;
        case OTA_RECEIVE_ERROR:
            line2 = "Receive Failed";
            break;
        case OTA_END_ERROR:
            line2 = "End Failed";
            break;
        }
        Serial.println(line2);
    });

    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(HOSTNAME);

    ArduinoOTA.begin();
}

//------------------------------------------------------------------------
void connectToWifi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

//------------------------------------------------------------------------
void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();

    MainDisplay::setup();
}

//------------------------------------------------------------------------
void publishSensorValue()
{
    String val = String(254.0 / 4095.0 * (4095 - analogRead(LDR_PIN)));
    mqttClient.publish("ESPClock/ldr/value", 0, true, val.c_str());
}

//------------------------------------------------------------------------
void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");

    // attach the publisher
    mqttPublishTimer.attach(PUBLISH_FREQ_SECS, publishSensorValue);

    mqttClient.subscribe("ESPClock/Set/Alarm", 0);
}

//------------------------------------------------------------------------
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");

    mqttPublishTimer.detach(); // stop publishing

    if (WiFi.isConnected())
        mqttReconnectTimer.once(2, connectToMqtt);
}
//------------------------------------------------------------------------
bool HasErrored(DeserializationError error)
{
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return true;
    }
    return false;
}
//------------------------------------------------------------------------
/*
{
  "Index": 0,
  "Hour": 8,
  "Minute": 8,
  "Active": true,
  "Days": "0111110"
}
*/
//------------------------------------------------------------------------
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    if (strcmp("ESPClock/Set/Alarm", topic) == 0)
    {
        const size_t capacity = JSON_OBJECT_SIZE(5) + 38;
        DynamicJsonDocument jsonDoc(capacity);

Serial.println(payload);

        if (HasErrored(deserializeJson(jsonDoc, payload)))
            return;
        
        int index = jsonDoc["Index"];
        if( index < 10)
        {
            Alarms[index].init(jsonDoc["Hour"], jsonDoc["Minute"], jsonDoc["Days"] );
        }
        return;
    }
}
//------------------------------------------------------------------------
void init_mqtt()
{
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    //mqttClient.onSubscribe(onMqttSubscribe);
    //mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    //mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}
//------------------------------------------------------------------------
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        mqttReconnectTimer.detach();
        wifiReconnectTimer.once(2, connectToWifi);
        break;
    case SYSTEM_EVENT_WIFI_READY:    /**< ESP32 WiFi ready */
    case SYSTEM_EVENT_SCAN_DONE:     /**< ESP32 finish scanning AP */
    case SYSTEM_EVENT_STA_START:     /**< ESP32 station start */
    case SYSTEM_EVENT_STA_STOP:      /**< ESP32 station stop */
    case SYSTEM_EVENT_STA_CONNECTED: /**< ESP32 station connected to AP */
    //case SYSTEM_EVENT_STA_DISCONNECTED:         /**< ESP32 station disconnected from AP */
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: /**< the auth mode of AP connected by ESP32 station changed */
    //case SYSTEM_EVENT_STA_GOT_IP:               /**< ESP32 station got IP from connected AP */
    case SYSTEM_EVENT_STA_LOST_IP:        /**< ESP32 station lost IP and the IP is reset to 0 */
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: /**< ESP32 station wps succeeds in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:  /**< ESP32 station wps fails in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT: /**< ESP32 station wps timeout in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_PIN:     /**< ESP32 station wps pin code in enrollee mode */
    case SYSTEM_EVENT_AP_START:           /**< ESP32 soft-AP start */
    case SYSTEM_EVENT_AP_STOP:            /**< ESP32 soft-AP stop */
    case SYSTEM_EVENT_AP_STACONNECTED:    /**< a station connected to ESP32 soft-AP */
    case SYSTEM_EVENT_AP_STADISCONNECTED: /**< a station disconnected from ESP32 soft-AP */
    case SYSTEM_EVENT_AP_STAIPASSIGNED:   /**< ESP32 soft-AP assign an IP to a connected station */
    case SYSTEM_EVENT_AP_PROBEREQRECVED:  /**< Receive probe request packet in soft-AP interface */
    case SYSTEM_EVENT_GOT_IP6:            /**< ESP32 station or ap or ethernet interface v6IP addr is preferred */
    case SYSTEM_EVENT_ETH_START:          /**< ESP32 ethernet start */
    case SYSTEM_EVENT_ETH_STOP:           /**< ESP32 ethernet stop */
    case SYSTEM_EVENT_ETH_CONNECTED:      /**< ESP32 ethernet phy link up */
    case SYSTEM_EVENT_ETH_DISCONNECTED:   /**< ESP32 ethernet phy link down */
    case SYSTEM_EVENT_ETH_GOT_IP:         /**< ESP32 ethernet got IP from connected AP */
    default:
        break;
    }
}
//------------------------------------------------------------------------
void init_Display()
{
    tft.init();
    tft.setRotation(3);

    tft.setTextColor(TFT_BLUE, TFT_BLACK); // Note: the new fonts do not draw the background colour
    tft.setTextFont(4);
    tft.print("WiFi");

    // init Backlight PWM
    ledcSetup(BACKLIGHT_CHANNEL, 4000, 8);
    ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
    ledcWrite(BACKLIGHT_CHANNEL, 255);

    // setup the display brightness
    ldrTimer.attach(1, CheckLDR);
}
//------------------------------------------------------------------------
void setup(void)
{
    Serial.begin(115200);

    // set pins
    pinMode(LDR_PIN, INPUT);

    // init the TFT Display
    init_Display();

    WiFi.onEvent(WiFiEvent);
    //init_OTA();
    init_mqtt();
    connectToWifi();

    // Init Beeper
    ledcSetup(TONE_CHANNEL, 2700, 8);

    Beep(TONE_FREQ, 75);
    delay(75);
    Beep(TONE_FREQ, 75);
}

//------------------------------------------------------------------------
void CheckAlarms(TButtonEvent left, TButtonEvent right)
{
    for(int idx = 0; idx < 10; idx++)
        Alarms[idx].Check(left, right);
}
//------------------------------------------------------------------------
void loop()
{
    CheckAlarms(Left.Check(),Right.Check());

    delay(10);
}
