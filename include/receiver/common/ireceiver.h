#pragma once

#include "base/complex.h"

#include <cstdint>
#include <functional>
#include <vector>

using namespace Base;

struct BaseSettingsReceiver {
    virtual ~BaseSettingsReceiver() = default;
};

struct ReceiverSettings : public BaseSettingsReceiver {
    int direct_sampling{0};
    int gain{0};
    int ppm_error{0};
    int agcMode{1};
    uint32_t sampleCount;
    uint32_t centralFreq; // fc
    uint32_t sampleFreq;  // fd

    ReceiverSettings(uint32_t centralFreq, uint32_t sampleFreq) : centralFreq(centralFreq), sampleFreq(sampleFreq) { }
    ~ReceiverSettings() override = default;
};

struct sinParams {
    float amp;
    uint64_t freq;
};

struct fakeParams : public BaseSettingsReceiver {
    uint32_t fd;
    uint32_t sampleCount;
    float noiseLVL;
    std::vector<sinParams> sinPar;

    ~fakeParams() override = default;
};

struct BaseSettingTransaction {
    virtual ~BaseSettingTransaction() { }
};

enum class TypeTransaction { single, loop };
struct SettingTransaction : BaseSettingTransaction {
    SettingTransaction(size_t bufferSize, TypeTransaction typeTransaction, size_t ircSize, bool testMode) :
        bufferSize(bufferSize), typeTransaction(typeTransaction), ircSize(ircSize), testMode(testMode) { }
    SettingTransaction() { }

    std::size_t bufferSize{0};
    TypeTransaction typeTransaction{0};
    std::size_t ircSize{0};
    bool testMode = false;
};

class IReceiver {
public:
    IReceiver();
    virtual ~IReceiver() = default;

    virtual void setSettingsReceiver(BaseSettingsReceiver* sett)                = 0;
    virtual void setSettingsTransaction(BaseSettingTransaction* sett)           = 0;
    virtual void setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) = 0;

    virtual void start() = 0;
    virtual void stop()  = 0;

protected:
    virtual bool getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) = 0;
    bool isNeedProcessing() {
        return needProcessing;
    }

    bool needProcessing = true;

    std::function<void(Complex<int8_t>*, uint32_t)> process;
    std::vector<Complex<int8_t> > complexBuff;
};

inline IReceiver::IReceiver() {
    // complexBuff.reserve(settingTransaction.bufferSize);
}
