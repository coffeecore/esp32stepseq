#pragma once
 
#include "PlayNoteRequest.h"
#include "VoiceHandle.h"



class IAudioEngine
{
    public:
        virtual ~IAudioEngine() = default;
 
        virtual void begin() = 0;
 
        virtual VoiceHandle play(const PlayNoteRequest& request) = 0;
 
        virtual void stop(VoiceHandle voice) = 0;
};
 