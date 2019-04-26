// todo: wifi setup
// location setting
//

#include "Shared.h"

#include "Button.h"
#include "Comms.h"
#include "Display.h"

#define TONE_CHANNEL 1
#define TONE_FREQ 2700

//------------------------------------------------------------------------
// button handlers
TButton Right(BUTTON_RIGHT_PIN, []() -> void IRAM_ATTR { Right.HandleEvent(); });
TButton Left(BUTTON_LEFT_PIN, []() -> void IRAM_ATTR { Left.HandleEvent(); });

//------------------------------------------------------------------------
void log(const String &msg)
{
    Serial.print(msg);
    PublishTopic("ESPClock/Log", false, msg.c_str());
}
//------------------------------------------------------------------------
void logln(const String &msg)
{
    Serial.println(msg);
    PublishTopic("ESPClock/Log", false, msg.c_str());
}

//------------------------------------------------------------------------
void Beep(int freq, int ms)
{
    pinMode(TONE_PIN, OUTPUT);
    ledcAttachPin(TONE_PIN, TONE_CHANNEL);

    ledcWriteTone(TONE_CHANNEL, freq);
    delay(ms);
    ledcWriteTone(TONE_CHANNEL, 0);

    ledcDetachPin(TONE_PIN);
    pinMode(TONE_PIN, INPUT);
}

//------------------------------------------------------------------------
void HandleMqttMessage(const char *topic, const char *payload)
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

//------------------------------------------------------------------------
void setup(void)
{
    Serial.begin(115200);

    TComms::setup();

    // Init Beeper
    ledcSetup(TONE_CHANNEL, TONE_FREQ, 8);

    Beep(TONE_FREQ, 75);
    delay(75);
    Beep(TONE_FREQ, 75);

    TDisplay::setup();
}
//------------------------------------------------------------------------
void loop()
{
    TButtonEvent leftBtn = Left.Check();
    TButtonEvent rightBtn = Right.Check();

    // allow the alarms to check for a cancel button
    for (int idx = 0; idx < 10; idx++)
        Alarms[idx].Check(leftBtn, rightBtn);

    TDisplay::HandleKeys(leftBtn, rightBtn);

    delay(10);
}
