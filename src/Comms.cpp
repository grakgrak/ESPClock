#include "Shared.h"
#include "Comms.h"
#include "..\..\Credentials.h" // contains definitions of WIFI SSID and password

#define MQTT_PORT 1883
#define MQTT_HOST IPAddress(192, 168, 1, 210)

#define PUBLISH_FREQ_SECS 60

#define HOSTNAME "ESPClock"
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

//------------------------------------------------------------------------
AsyncMqttClient mqttClient;

Ticker mqttReconnectTimer;
Ticker mqttPublishTimer;
Ticker wifiReconnectTimer;

String gPressure;
String gHumidity;
String gTemperature;
//------------------------------------------------------------------------
void connectToWifi()
{
    logln("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
}

//------------------------------------------------------------------------
void connectToMqtt()
{
    logln("Connecting to MQTT...");
    mqttClient.connect();
}

//------------------------------------------------------------------------
void publishSensorValue()
{
    if (gTemperature.length() > 0)
        mqttClient.publish("ESPClock/Temp/value", 0, true, gTemperature.c_str());
    if (gHumidity.length() > 0)
        mqttClient.publish("ESPClock/Humidity/value", 0, true, gHumidity.c_str());
    if (gPressure.length() > 0)
        mqttClient.publish("ESPClock/Pressure/value", 0, true, gPressure.c_str());
}
//------------------------------------------------------------------------
void PublishTopic( const String &topic, bool retain, const char *payload)
{
    mqttClient.publish(topic.c_str(), 0, retain, payload);
}

//------------------------------------------------------------------------
void onMqttConnect(bool sessionPresent)
{
    logln("Connected to MQTT.");

    // attach the publisher
    mqttPublishTimer.attach(PUBLISH_FREQ_SECS, publishSensorValue);

    // subscribe to the alarm topics
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        String tmp = "ESPClock/Set/Alarm" + String(i);
        mqttClient.subscribe(tmp.c_str(), 0);
    }
}

//------------------------------------------------------------------------
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    logln("Disconnected from MQTT.");

    mqttPublishTimer.detach(); // stop publishing

    if (WiFi.isConnected())
        mqttReconnectTimer.once(2, connectToMqtt);
}
//------------------------------------------------------------------------
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    HandleMqttMessage(topic, payload);
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
    logln("[WiFi-event] event: " + String(event));
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        logln("WiFi connected IP: " + WiFi.localIP().toString());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        logln("WiFi lost connection");
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
void TComms::setup()
{
    WiFi.onEvent(WiFiEvent);
    init_mqtt();
    connectToWifi();
}
