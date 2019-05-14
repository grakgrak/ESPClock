#include "Shared.h"
#include "KeyboardDisplay.h"
#include "Display.h"

// BackSpace, Shift, Ok
//const char *keys = "0123456789 aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
 const char *keys[] = {
     "0123456789abcdefghijklmnopqrstuvwxyz ",
     "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ",
     ",.:;%^&(){}[]_*-+=\\|/<>?~!@"};

#define COL_WIDTH 24
#define ROW_HEIGHT 30
#define COL_COUNT 10
#define ROW_COUNT 7
#define EXTRA   3
int charSet = 0;
int cursorIdx = 0;
int maxKeys = 0;
String result;

//------------------------------------------------------------------------
void KeyboardDisplay::start()
{
    Display::BlankArea(0, TFT_HEIGHT);

    charSet = 0;
    cursorIdx = 0;
    maxKeys = strlen(keys[charSet]) + EXTRA;
    result = "";

    DrawKeyboard();
}

//------------------------------------------------------------------------
void KeyboardDisplay::stop()
{
}
//------------------------------------------------------------------------
bool KeyboardDisplay::keypress(TButtonEvent left, TButtonEvent right)
{
    if (left == TButtonEvent::PRESS)
    {
        if (cursorIdx > 0)
            --cursorIdx;
        else
            cursorIdx = maxKeys - 1;

        DrawKeyboard();
        return true;
    }

    if (right == TButtonEvent::PRESS)
    {
        if (cursorIdx < maxKeys - 1 )   // allow for the extra buttons
            ++cursorIdx;
        else
            cursorIdx = 0;
        

        DrawKeyboard();
        return true;
    }

    if (left == TButtonEvent::LONG_PRESS || right == TButtonEvent::LONG_PRESS)
    {
        if( cursorIdx < maxKeys - EXTRA)
            result = result + String(keys[charSet][cursorIdx]);

        if(cursorIdx == maxKeys - EXTRA)
        {
            charSet = (charSet + 1) % 3;
            maxKeys = strlen(keys[charSet]);
            cursorIdx = maxKeys - EXTRA;
            Display::BlankArea(0, TFT_HEIGHT);
        }

        if(cursorIdx == maxKeys - EXTRA + 1)
            result = result.substring(0, result.length() - 1);

        if(cursorIdx == maxKeys - EXTRA + 2)
            return false;

        DrawKeyboard();
        return true;
    }

    return false;
}

//------------------------------------------------------------------------
void KeyboardDisplay::DrawKeyboard()
{
    auto tft = Display::Lock();

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // top left

    // adjust cursorIdx so that it is valid
    // render the keyboard
    for (int r = 0; r < ROW_COUNT; r++)
        for (int c = 0; c < COL_COUNT; c++)
        {
            int idx = r * COL_COUNT + c;
            if (idx >= maxKeys - EXTRA)
                break;

            int background = TFT_BLACK;
            if (idx == cursorIdx)
                background = TFT_RED;

            tft.setTextColor(TFT_WHITE, background);
            tft.drawChar(keys[charSet][idx], c * COL_WIDTH, r * ROW_HEIGHT, 4);
        }

    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.fillRect(0,TFT_HEIGHT - ROW_HEIGHT * 3, TFT_WIDTH, ROW_HEIGHT, TFT_BLACK);
    tft.drawString(result, 0, TFT_HEIGHT - ROW_HEIGHT * 3, 4);

    tft.setTextColor(TFT_WHITE, (cursorIdx == maxKeys - EXTRA) ? TFT_RED : TFT_BLACK);
    tft.drawString("Shift", 0, TFT_HEIGHT - ROW_HEIGHT, 4);
    tft.setTextColor(TFT_WHITE, (cursorIdx == maxKeys - EXTRA + 1) ? TFT_RED : TFT_BLACK);
    tft.drawString("BSpace", 80, TFT_HEIGHT - ROW_HEIGHT, 4);
    tft.setTextColor(TFT_WHITE, (cursorIdx == maxKeys - EXTRA + 2) ? TFT_RED : TFT_BLACK);
    tft.drawString("Ok", 190, TFT_HEIGHT - ROW_HEIGHT, 4);

    Display::Release();
}