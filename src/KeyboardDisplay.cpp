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

int charSet;
int cursorIdx;
bool rowSelectMode;
String result;
//------------------------------------------------------------------------
void KeyboardDisplay::setup()
{
}

//------------------------------------------------------------------------
void KeyboardDisplay::start()
{
    TDisplay::BlankArea(0, TFT_HEIGHT);

    charSet = 0;
    cursorIdx = 0;
    rowSelectMode = true;
    result = "";

    DrawKeyboard(0,0);
}

//------------------------------------------------------------------------
void KeyboardDisplay::stop()
{
}
//------------------------------------------------------------------------
bool KeyboardDisplay::keypress(TButtonEvent left, TButtonEvent right)
{
    int r = cursorIdx / COL_COUNT;
    int c = cursorIdx % COL_COUNT;

    if (left == TButtonEvent::PRESS)
    {
        if (rowSelectMode)
        {
            if (r > 0)
                --r;
        }
        else if (c > 0)
            --c;

        DrawKeyboard(r, c);
        return true;
    }

    if (left == TButtonEvent::LONG_PRESS)
    {
        rowSelectMode = (rowSelectMode == false);

        DrawKeyboard(r, c);
        return true;
    }

    if (right == TButtonEvent::LONG_PRESS)
    {
        result = result + String(keys[0][cursorIdx]);
        DrawKeyboard(r, c);
        return true;
    }

    if (right == TButtonEvent::PRESS)
    {
        if (rowSelectMode)
        {
            if (r < ROW_COUNT - 1)
                ++r;
        }
        else if (c < COL_COUNT - 1)
            ++c;

        DrawKeyboard(r, c);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
void KeyboardDisplay::DrawKeyboard(int r, int c)
{
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // top left

    // adjust cursorIdx so that it is valid
    cursorIdx = r * COL_COUNT + c;
    if (cursorIdx >= strlen(keys[0]))
        cursorIdx = strlen(keys[0]) - 1;

    // render the keyboard
    for (int r = 0; r < ROW_COUNT; r++)
        for (int c = 0; c < COL_COUNT; c++)
        {
            int idx = r * COL_COUNT + c;
            if (idx >= strlen(keys[0]))
                break;

            int background = TFT_BLACK;
            if (rowSelectMode)
            {
                if (r == cursorIdx / COL_COUNT)
                    background = TFT_RED;
            }
            else
            {
                if (idx == cursorIdx)
                    background = TFT_RED;
            }

            tft.setTextColor(TFT_WHITE, background);
            tft.drawChar(keys[0][idx], c * COL_WIDTH, r * ROW_HEIGHT, 4);
        }

    tft.drawString(result, 0, TFT_HEIGHT - ROW_HEIGHT * 2, 4);

    tft.drawString("Shift", 0, TFT_HEIGHT - ROW_HEIGHT, 4);
    tft.drawString("BSpace", 80, TFT_HEIGHT - ROW_HEIGHT, 4);
    tft.drawString("Ok", 190, TFT_HEIGHT - ROW_HEIGHT, 4);
}