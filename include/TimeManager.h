#pragma once

#include <Arduino.h>

class TimeManager {
public:
    void init();
    void update();
    void syncTime();
    
    int getHour();
    int getMinute();
    int getSecond();
    String getTimeString();
    
    bool isTimeSet();

private:
    bool _timeSet = false;
    unsigned long _lastSyncTime = 0;
};
