#include "Shared.h"
#include "Display.h"
#include "AlarmDisplay.h"
#include "Comms.h"

namespace AlarmDisplay
{
Ticker alarmTimer;
TAlarm Alarms[MAX_ALARMS];

//------------------------------------------------------------------------
void AddMqttSubscribeTopics()
{
    // subscribe to the alarm topics
    for (int i = 0; i < MAX_ALARMS; ++i)
    {
        String tmp = "ESPClock/Set/Alarm" + String(i);
        Comms::SubscribeTopic(tmp);
    }
}
//------------------------------------------------------------------------
void HandleMqttMessage(const char *topic, const char *payload)
{
    // check for alarm related messages
    if (strncmp("ESPClock/Set/", topic, 13) == 0)
    {
        // check for a command to set the alarm
        for (int i = 0; i < MAX_ALARMS; ++i)
        {
            String tmp = "ESPClock/Set/Alarm" + String(i);
            if (strcmp(tmp.c_str(), topic) == 0)
            {
                Alarms[i].JsonUpdate(payload);
                return;
            }
        }
    }
}

//------------------------------------------------------------------------
void DrawAlarms()
{
    auto tft = Display::Lock();

    // blank the display area
    Display::BlankArea(CLOCK_HEIGHT, TFT_HEIGHT);

    // render the list of Alarms
    tft.setTextSize(1); // reset the text size
    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    // list the alarms
    for (int i = 0; i < MAX_ALARMS; ++i)
        if (Alarms[i].isEnabled())
            tft.drawString(Alarms[i].toString(), 0, CLOCK_HEIGHT + (i * LINE_HEIGHT), 4);
        else
        {
            int top = CLOCK_HEIGHT + (i * LINE_HEIGHT);
            Display::BlankArea(top, top + LINE_HEIGHT);
        }

    Display::Release();
}
//------------------------------------------------------------------------
void Check(TButtonEvent left, TButtonEvent right)
{
    // allow the alarms to check for a cancel button
    for (int idx = 0; idx < 10; idx++)
        Alarms[idx].Check(left, right);
}
//------------------------------------------------------------------------
void start()
{
    logln("start AlarmDisplay");
    // setup the timers
    alarmTimer.attach(10, DrawAlarms);

    DrawAlarms();
}
//------------------------------------------------------------------------
void stop()
{
    logln("stop AlarmDisplay");
    alarmTimer.detach();
}
} // namespace AlarmDisplay