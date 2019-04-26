#include "Shared.h"
#include "AlarmDisplay.h"

Ticker alarmTimer;

//------------------------------------------------------------------------
void AlarmDisplay::setup()
{

}
//------------------------------------------------------------------------
void AlarmDisplay::start()
{
    logln("start AlarmDisplay");
    // setup the timers
    alarmTimer.attach(10, DrawAlarms);

    DrawAlarms();
}
//------------------------------------------------------------------------
void AlarmDisplay::stop()
{
    logln("stop AlarmDisplay");
    alarmTimer.detach();
}
//------------------------------------------------------------------------
void AlarmDisplay::DrawAlarms()
{
    // blank the display area
    TDisplay::BlankArea(CLOCK_HEIGHT, TFT_HEIGHT);

    // render the list of Alarms
    tft.setTextSize(1); // reset the text size
    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    // list the alarms
    for(int i = 0; i < MAX_ALARMS; ++i)
        if( Alarms[i].isEnabled())
            tft.drawString(Alarms[i].toString(), 0, CLOCK_HEIGHT + (i * LINE_HEIGHT), 4);
        else
        {
            int top = CLOCK_HEIGHT + (i * LINE_HEIGHT);
            TDisplay::BlankArea(top, top + LINE_HEIGHT);
        }

}
