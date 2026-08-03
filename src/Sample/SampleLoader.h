#pragma once

#include "Arduino.h"
#include "Constants.h"

struct MySample
{
    uint16_t id;
    const char* name;
    const char* path;

    const int16_t* data;
    uint32_t length;
    uint32_t rate;
};

class SampleLoader
{
public:
    MySample* samples = nullptr;
    size_t sampleTotal = 0;
    size_t sampleCount = 0;

    void begin()
    {
        init(5);
    }

    void init(size_t nbFiles)
    {
        sampleTotal = nbFiles;
        samples = new MySample[sampleTotal];
    }

    ~SampleLoader()
    {
        delete[] samples;
    }

    MySample* getSampleById(uint16_t sampleId)
    {
        for (size_t i = 0; i < sampleCount; ++i) {
            if (samples[i].id == sampleId)
                return &samples[i];
        }

        return nullptr;
    }

    void addSample(uint16_t sampleId, const char* name, const char* path, const int16_t* data, uint32_t length,
                   uint32_t rate)
    {
        if (sampleCount >= sampleTotal) {
            return; // ou afficher une erreur
        }

        samples[sampleCount] = {sampleId, name, path, data, length, rate};

        sampleCount++;
    }

    const int16_t* getData(uint16_t sampleId)
    {
        MySample* sample = getSampleById(sampleId);

        if (sample == nullptr) {
            return nullptr;
        }

        return sample->data;
    }

    uint32_t getLength(uint16_t sampleId)
    {
        MySample* sample = getSampleById(sampleId);

        if (sample == nullptr) {
            return 0;
        }

        return sample->length;
    }

    uint32_t getRate(uint16_t sampleId)
    {
        MySample* sample = getSampleById(sampleId);

        if (sample == nullptr) {
            return 0;
        }

        return sample->rate;
    }
};
