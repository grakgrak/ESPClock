#pragma once
#include <Arduino.h>

namespace Clock
{
int dstOffset(bool init = false);

int getHours();
int getMinutes();
int getDay();

bool update();  // call frequently
void setup();
} // namespace Clock