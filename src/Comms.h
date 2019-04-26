#pragma once


class TComms
{
public:
    static void setup();

    void PublishTopic( const String &topic, bool retain, const char *payload);

};