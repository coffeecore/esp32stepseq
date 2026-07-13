#pragma once

#include "Arduino.h"
#include "Constants.h"

class HTimer
{
    public:
        using TimerCallback = void (*)();

        HTimer()
        {
        }

        void begin(uint64_t _tickDurationInMicroSeconds)
        {
            tickDurationInMicroSeconds = _tickDurationInMicroSeconds;

            Serial.println("Start timer");
            globalInstance = this;
            timer = timerBegin(1000000);
            timerAttachInterrupt(timer, &HTimer::onTimer);
            Serial.println("SET ALARM");
            timerAlarm(timer, tickDurationInMicroSeconds, true, 0);
            Serial.println("Started timer");

        }

        void setCallback(TimerCallback cb)
        {
            callback = cb;
        }

        void setTickDurationInMicroSeconds(uint64_t _tickDurationInMicroSeconds)
        {
            tickDurationInMicroSeconds = _tickDurationInMicroSeconds;
            Serial.println("SET ALARM");
            timerAlarm(timer, tickDurationInMicroSeconds, true, 0);
        }

    private:
        hw_timer_t* timer = nullptr;
        static HTimer* globalInstance;
        uint64_t tickDurationInMicroSeconds;
        TimerCallback callback = nullptr;

        static void IRAM_ATTR onTimer()
        {
            if (globalInstance == nullptr) {
                return;
            }

            if (globalInstance->callback) {
                globalInstance->callback();
            }
        }
};

HTimer* HTimer::globalInstance = nullptr;
