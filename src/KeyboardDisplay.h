#pragma once

class KeyboardDisplay
{
private:
    static void DrawKeyboard();
public:
    static void start();
    static void stop();
    static bool keypress(TButtonEvent left, TButtonEvent right);
};