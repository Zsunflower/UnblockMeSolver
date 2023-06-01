#ifndef CLOCK_H
#define CLOCK_H

#include <string>

class Clock
{
private:
    int hour, minute, second;

public:
    Clock();
    void init();
    void tick();
    std::string getFormatCurrentTime();
};

#endif
