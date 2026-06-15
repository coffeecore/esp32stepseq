#pragma once

#include "Arduino.h"
#include "Constants.h"
// #include "SequencerTimer.h"

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
            timer = timerBegin(0, 80, true);
            timerAttachInterrupt(timer, &HTimer::onTimer, true);
            timerAlarmWrite(timer, tickDurationInMicroSeconds, true); // 1000 µs = 1 ms
            timerAlarmEnable(timer);
            Serial.println("Started timer");

        }

        void setCallback(TimerCallback cb)
        {
            callback = cb;
        }

        void setTickDurationInMicroSeconds(uint64_t _tickDurationInMicroSeconds)
        {
            tickDurationInMicroSeconds = _tickDurationInMicroSeconds;

            timerAlarmDisable(timer);
            timerWrite(timer, 0); // optionnel : remet le compteur à zéro
            timerAlarmWrite(timer, tickDurationInMicroSeconds, true);
            timerAlarmEnable(timer);
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
