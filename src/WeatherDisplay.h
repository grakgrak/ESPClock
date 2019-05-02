#pragma once

namespace WeatherDisplay
{
void start();
void stop();

void AddMqttSubscribeTopics();
void HandleMqttMessage(const char *topic, const char *payload);
void HandleKeys(TButtonEvent left, TButtonEvent right);
} // namespace WeatherDisplay