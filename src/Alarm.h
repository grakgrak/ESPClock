#pragma once

#include <Arduino.h>
#include <NTPClient.h>
#include "Button.h"

extern void Beep(int freq, int ms);

//-------------------------------------------------------------
class TAlarm
{
  private:
    enum AlarmState
    {
        INACTIVE,
        WAITING,
        FIRED,
        SNOOZE,
        CANCELLED
    };
   
    AlarmState _state;
    unsigned _hour;
    unsigned _minute;
    String _days;
    int _index;
  public:
    //-------------------------------------------------------------
    bool isEnabled() { return _state != AlarmState::INACTIVE; }

    String toString();

    void init(int index, unsigned hour, unsigned mins, const String &days);
    bool Check(TButtonEvent left, TButtonEvent right);

    void JsonUpdate(const char *payload);
};
