#pragma once

class KeyboardDisplay
{
private:
    static void DrawKeyboard(int r, int c);
public:
    static void setup();
    static void start();
    static void stop();
    static bool keypress(TButtonEvent left, TButtonEvent right);
};