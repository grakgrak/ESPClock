#pragma once

#include <Arduino.h>
#include <NTPClient.h>
#include "Button.h"

extern NTPClient timeClient;
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

  public:
    //-------------------------------------------------------------
    void init(unsigned hour, unsigned mins, const String &days)
    {
        _state = AlarmState::WAITING;
        _hour = hour;
        _minute = mins;
        _days = (days == "") ? "0000000" : days;
        Serial.println("Alarm Init");
    }

    //-------------------------------------------------------------
    void Check(TButtonEvent left, TButtonEvent right)
    {
        unsigned alarmMinutes = _hour * 60 + _minute;
        unsigned nowMinutes = timeClient.getHours() * 60 + timeClient.getMinutes();

        switch (_state)
        {
        case AlarmState::INACTIVE:
            break;
        case AlarmState::WAITING:
            //check we are once only or on a valid day
            if (_days == "0000000" || _days[timeClient.getDay()] == '1')
                if (nowMinutes == alarmMinutes)
                    _state = AlarmState::FIRED;
            break;
        case AlarmState::FIRED:
            if (left == TButtonEvent::PRESS || right == TButtonEvent::PRESS)
                _state = AlarmState::CANCELLED;
            if (left == TButtonEvent::LONG_PRESS || right == TButtonEvent::LONG_PRESS)
                _state = AlarmState::CANCELLED;
            break;
        case AlarmState::SNOOZE:
            break;
        case AlarmState::CANCELLED:
            if (_days == "0000000") // only fire once
                _state = AlarmState::INACTIVE;
            else if (nowMinutes != alarmMinutes)
                _state = AlarmState::WAITING;
            break;
        }

        if (_state == AlarmState::FIRED)
        {
            Beep(2700, 50);
            delay(100);
            if( nowMinutes - alarmMinutes >= 2)
                _state = AlarmState::CANCELLED;
        }
        //todo: Handle snooze
    }
};
