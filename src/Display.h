#pragma once
#include "Button.h"

namespace Display
{
TFT_eSPI &Lock();
void Release();

void BlankArea(int top, int bottom);
void HandleKeys(TButtonEvent left, TButtonEvent right);
void setup();
} // namespace Display