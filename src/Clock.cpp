#include "Clock.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

namespace Clock
{
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 60000);

struct strDateTime
{
    byte hour;
    byte minute;
    byte second;
    int year;
    byte month;
    byte day;
    byte dayofWeek;
    unsigned long epochTime;
};

strDateTime _now;

//------------------------------------------------------------------------
#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))

static const uint8_t _monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int DST = 0;

//------------------------------------------------------------------------
strDateTime ConvertUnixTimestamp(unsigned long timeStamp)
{
    strDateTime dateTime;

    dateTime.epochTime = timeStamp;

    uint32_t _time = (uint32_t)timeStamp;
    dateTime.second = _time % 60;
    _time /= 60; // now it is minutes
    dateTime.minute = _time % 60;
    _time /= 60; // now it is hours
    dateTime.hour = _time % 24;
    _time /= 24;                                // now it is _days
    dateTime.dayofWeek = ((_time + 4) % 7) + 1; // Sunday is day 1

    uint8_t _year = 0;
    unsigned long _days = 0;
    while ((unsigned)(_days += (LEAP_YEAR(_year) ? 366 : 365)) <= _time)
        _year++;

    dateTime.year = _year; // year is offset from 1970

    _days -= LEAP_YEAR(_year) ? 366 : 365;
    _time -= _days; // now it is days in this year, starting at 0

    uint8_t _month = 0;
    uint8_t _monthLength = 0;
    for (_month = 0; _month < 12; _month++)
    {
        if (_month == 1) // february
            _monthLength = LEAP_YEAR(_year) ? 29 : 28;
        else
            _monthLength = _monthDays[_month];

        if (_time >= _monthLength)
            _time -= _monthLength;
        else
            break;
    }

    dateTime.month = _month + 1; // jan is month 1
    dateTime.day = _time + 1;    // day of month
    dateTime.year += 1970;

    return dateTime;
}

//------------------------------------------------------------------------
int getHours()
{
    return timeClient.getHours();
}
//------------------------------------------------------------------------
int getMinutes()
{
    return timeClient.getMinutes();
}
//------------------------------------------------------------------------
int getDay()
{
    return timeClient.getDay();
}

//------------------------------------------------------------------------
bool isSummerTime()
{
    if (_now.month < 3 || _now.month > 10)
        return false;
    if (_now.month > 3 && _now.month < 10)
        return true;
    if (_now.month == 3)
        if ((_now.hour + 24 * _now.day) >= (3 + 24 * (31 - (5 * _now.year / 4 + 4) % 7)))
            return true;
    if (_now.month == 10)
        if ((_now.hour + 24 * _now.day) < (3 + 24 * (31 - (5 * _now.year / 4 + 1) % 7)))
            return true;
    return false;
}
//------------------------------------------------------------------------
int dstOffset(bool init)
{
    static int DST = 0;

    if (init)
        DST = isSummerTime() ? 3600 : 0;

    // last sunday in october
    if (_now.dayofWeek == 0 && _now.month == 10 && _now.day >= 25 && _now.hour == 3 && DST > 0)
        DST = 0;

    // last sunday in march
    if (_now.dayofWeek == 0 && _now.month == 3 && _now.day >= 25 && _now.hour == 2 && DST == 0)
        DST = 3600;

    return DST;
}

//------------------------------------------------------------------------
bool update()
{
    if (timeClient.update() == false)
        return false;

    unsigned long unixTime = timeClient.getEpochTime();

    if (unixTime - _now.epochTime > 10) // every 10 seconds
    {
        bool initDST = _now.epochTime == 0;

        _now = Clock::ConvertUnixTimestamp(unixTime);

        timeClient.setTimeOffset(dstOffset(initDST));
    }
    return true;
}

//------------------------------------------------------------------------
void setup()
{
    _now.epochTime = 0;
}

} // namespace Clock