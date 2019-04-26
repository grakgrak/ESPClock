#pragma once

#include <Arduino.h>

extern void Beep(int freq, int ms);

enum class TButtonEvent { NONE, PRESS, LONG_PRESS };
class TButton
{
public:
    //-------------------------------------------------
    TButton(int pin, void (*callback)())
    {
        _pin = pin;
        pinMode(pin, INPUT);
        attachInterrupt(digitalPinToInterrupt(pin), callback, CHANGE);
    }
    //-------------------------------------------------
    void HandleEvent()
    {
        if( digitalRead(_pin))
        {
            _pressed = true;
            _start = millis();
        }
        else
            _released = true;
    }
    //-------------------------------------------------
    TButtonEvent Check()
    {
        if (_pressed)
        {
            // check for long press
            if( millis() - _start > 750)
            {
                _pressed = false;
                Beep(2700, 50);
                return TButtonEvent::LONG_PRESS;
            }
        }

        if( _released )
        {
            _released = false;
            if( _pressed )
            {
                Beep(2700, 5);
                _pressed = false;
                return TButtonEvent::PRESS;
            }
        }
        return TButtonEvent::NONE;
    }
private:
    int _pin;
    unsigned long _start;
    bool _pressed;
    bool _released;
    void buttonEvent();
};
