#pragma once

namespace Comms
{
void setup();
void PublishTopic(const String &topic, bool retain, const char *payload);
void SubscribeTopic(const String &topic);
} // namespace Comms