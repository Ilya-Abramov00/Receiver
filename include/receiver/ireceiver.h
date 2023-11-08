#pragma once

#include "base/complex.h"
#include <cstdint>
#include <vector>

using namespace Base;

struct BaseSettings {

    uint32_t sampleCount;
    virtual ~BaseSettings() = default;
};

struct RfSettings {
    uint32_t centralFreq; // fc
    uint32_t sampleFreq; // fd
    int gain { 0 };
    int ppm_error { 0 };
    int agcMode { 1 };
};

struct ReceiverSettings: public BaseSettings {
    RfSettings rfSettings;
    // uint32_t sampleCount; //size
    int n_read;
    int sync_mode { 1 };
    int direct_sampling { 0 };
    int dithering { 1 };

    ~ReceiverSettings() override = default;

};

struct sinParams {
    float amp;
    uint64_t freq;
};

struct fakeParams: public BaseSettings {
    uint32_t fd;
    // uint64_t size;
    float noiseLVL;
    std::vector< sinParams > sinPar;

    ~fakeParams() override = default;
};




class IReceiver {
public:
    using Buffer = std::vector< Complex< uint8_t > >;
    using SpectBuff = std::vector< Complex< double > >;
    virtual ~IReceiver() = default;

    virtual void setSettings(  BaseSettings* sett )  = 0;//
    virtual bool getComplex( const BaseSettings* sett, Buffer& ) = 0;
    virtual void getSpectrum( const BaseSettings* sett, SpectBuff& ) = 0;

    virtual bool getComplex(  Buffer& ) = 0;
    virtual void getSpectrum(  SpectBuff& ) = 0;
};


