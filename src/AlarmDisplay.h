#pragma once

class AlarmDisplay
{
private:
    static void DrawAlarms();
public:  
    static void setup();
    static void start();
    static void stop();
};