#pragma once

namespace AlarmDisplay
{
void start();
void stop();

void Check(TButtonEvent left, TButtonEvent right);

void AddMqttSubscribeTopics();
void HandleMqttMessage(const char *topic, const char *payload);
} // namespace AlarmDisplay