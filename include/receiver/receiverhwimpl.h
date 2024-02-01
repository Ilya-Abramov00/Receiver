#pragma once

#include "ireceiver.h"
#include "rtl-sdr.h"

#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>
class ReceiverHWImpl : public IReceiver {
public:
    ReceiverHWImpl(SettingTransaction settingTransaction);
    ~ReceiverHWImpl() override;

    virtual bool getComplex(const BaseSettings* settings, Buffer& out) override final;
    virtual void getSpectrum(const BaseSettings* settings, SpectBuff& out) override final;
    virtual void setSettings(BaseSettings* sett) override final;

    void start() override final;
    void stop() override final;
    virtual bool getComplex(Buffer& out) override final;
    virtual void getSpectrum(SpectBuff& out) override final;

    void setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) override final;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_d;
    rtlsdr_read_async_cb_t callback;
    std::unique_ptr<std::thread> thread;

    virtual bool getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) override final;
    void startLoop();
    void startSingle();
};
