#include "Shared.h"
#include "Alarm.h"
#include "AlarmDisplay.h"
#include "Clock.h"
#include "Comms.h"

//-------------------------------------------------------------
String TAlarm::toString()
{
    static String DayOfWeek[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    String hours(_hour);
    String mins(_minute);
    if (hours.length() == 1)
        hours = "0" + hours;
    if (mins.length() == 1)
        mins = "0" + mins;

    String days = "";

    if (_days == "0000000")
        days = "Once";
    else if (_days == "1111111")
        days = "Every day";
    else if (_days == "0111110")
        days = "Mon-Fri";
    else if (_days == "1000001")
        days = "Weekends";
    else
    {
        for (int i = 0; i < 7; ++i)
            if (_days[i] == '1')
                days += " " + DayOfWeek[i];
    }

    return hours + ":" + mins + " " + days;
}
//------------------------------------------------------------------------
bool HasErrored(DeserializationError error)
{
    if (error)
    {
        log(F("deserializeJson() failed: "));
        logln(error.c_str());
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
void TAlarm::JsonUpdate(const char *payload)
{
    if(payload == NULL)
    {
        // disable the alarm
        _hour = 0;
        _minute = 0;
        _days = "";
    }
    else
    {
        // process a json string
        const size_t capacity = JSON_OBJECT_SIZE(5) + 38;
        DynamicJsonDocument jsonDoc(capacity);

        if (HasErrored(deserializeJson(jsonDoc, payload)))
            return;

        _hour = jsonDoc["Hour"];
        _minute = jsonDoc["Minute"];
        _days = jsonDoc["Days"].as<String>();
    }
    
    if (_days.length() != 7)
        _state = AlarmState::INACTIVE;
    else
        _state = AlarmState::WAITING;
}

//------------------------------------------------------------------------
bool TAlarm::Check(TButtonEvent left, TButtonEvent right)
{
    unsigned alarmMinutes = _hour * 60 + _minute;
    unsigned nowMinutes = Clock::getHours() * 60 + Clock::getMinutes();

    switch (_state)
    {
    case AlarmState::INACTIVE:
        break;
    case AlarmState::WAITING:
        //check we are once only or on a valid day
        if (_days == "0000000" || _days[Clock::getDay()] == '1')
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
        {
            _state = AlarmState::INACTIVE;
            Comms::PublishTopic("ESPClock/Set/Alarm" + String(_index), false, NULL);   // delete the retained topic
        }
        else if (nowMinutes != alarmMinutes)
            _state = AlarmState::WAITING;
        break;
    }

    if (_state == AlarmState::FIRED)
    {
        Beep(2700, 50);
        delay(200);
        if (nowMinutes - alarmMinutes >= 2) // cancel after 2 mins
            _state = AlarmState::CANCELLED;
    }
    //todo: Handle snooze
    // return true if the
    return _state == AlarmState::FIRED;
}

//-------------------------------------------------------------
void TAlarm::init(int index, unsigned hour, unsigned mins, const String &days)
{
    _index = index;
    _hour = hour;
    _minute = mins;
    _days = days;

    if (days.length() != 7)
        _state = AlarmState::INACTIVE;
    else
        _state = AlarmState::WAITING;

    logln("Alarm Init: " + toString());
}
