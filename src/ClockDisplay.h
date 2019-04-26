#pragma once

class ClockDisplay
{
private:
    static void BlankDisplay();
    static void DrawTime();
public:  
    static void setup();
    static void start();
    static void stop();
};