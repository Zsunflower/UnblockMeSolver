#include "Clock.h"

Clock::Clock()
{
}

void Clock::init()
{
    hour = minute = second = 0;
}

void Clock::tick()
{
    ++second;
    if(second == 60)
    {
        second = 0;
        ++minute;
        if(minute == 60)
        {
            minute = 0;
            ++hour;
            if(hour == 24)
            {
                hour = minute = second = 0;
            }
        }
    }
}

std::string Clock::getFormatCurrentTime()
{
    char timeString[10];
    sprintf(timeString, "%02d:%02d:%02d", hour, minute, second);
    return std::string(timeString);
}
