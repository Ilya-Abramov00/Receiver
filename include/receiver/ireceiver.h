#pragma once

#include "base/complex.h"

#include <cstdint>
#include <functional>
#include <vector>

using namespace Base;

struct BaseSettings {
    uint32_t sampleCount;
    virtual ~BaseSettings() = default;
};

struct RfSettings {
    uint32_t centralFreq; // fc
    uint32_t sampleFreq;  // fd
    int gain{0};
    int ppm_error{0};
    int agcMode{1};
};

struct ReceiverSettings : public BaseSettings {
    RfSettings rfSettings;
    int n_read;
    int sync_mode{1};
    int direct_sampling{0};
    int dithering{1};

    explicit ReceiverSettings(const RfSettings& rfSettings) : rfSettings(rfSettings) { }
    ~ReceiverSettings() override = default;
};

struct sinParams {
    float amp;
    uint64_t freq;
};

struct fakeParams : public BaseSettings {
    uint32_t fd;
    // uint64_t size;
    float noiseLVL;
    std::vector<sinParams> sinPar;

    ~fakeParams() override = default;
};

const std::size_t maxBufferSize = 256 * 16384;

enum class TypeTransaction { single, loop };
struct SettingTransaction {
    std::size_t bufferSize{0};
    TypeTransaction typeTransaction{0};
    std::size_t ircSize{0};
    bool testMode = false;
};
class IReceiver {
public:
    using Buffer    = std::vector<Complex<uint8_t> >;
    using SpectBuff = std::vector<Complex<double> >;
    IReceiver(SettingTransaction settingTransaction);
    virtual ~IReceiver() = default;

    virtual void setSettings(BaseSettings* sett)                   = 0; //
    virtual bool getComplex(const BaseSettings* sett, Buffer&)     = 0;
    virtual void getSpectrum(const BaseSettings* sett, SpectBuff&) = 0;

    virtual void start()                                                        = 0;
    virtual void stop()                                                         = 0;
    virtual bool getComplex(Buffer&)                                            = 0;
    virtual void getSpectrum(SpectBuff&)                                        = 0;
    virtual void setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) = 0;

protected:
    virtual bool getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) = 0;
    bool isNeedProcessing() {
        return needProcessing;
    }

    bool needProcessing = true;
    SettingTransaction settingTransaction;

    std::function<void(Complex<int8_t>*, uint32_t)> process;
    std::vector<Complex<int8_t> > complexBuff;
};

inline IReceiver::IReceiver(SettingTransaction settingTransaction) : settingTransaction(settingTransaction) {
    complexBuff.reserve(settingTransaction.bufferSize);
}
