#pragma once

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <Ticker.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include "Display.h"
#include "Alarm.h"

//------------------------------------------------------------------------
#define BUTTON_LEFT_PIN 16
#define BUTTON_RIGHT_PIN 17
#define TONE_PIN 33
#define LDR_PIN 34
#define BACKLIGHT_PIN 26

//------------------------------------------------------------------------
#define MAX_ALARMS 4

#define CLOCK_HEIGHT 96
#define LINE_HEIGHT 30

//------------------------------------------------------------------------
// common routines in main.cpp
extern void log(const String &msg);
extern void logln(const String &msg);
extern void HandleMqttMessage(const char *topic, const char *payload);
extern void AddMqttSubscribeTopics();
