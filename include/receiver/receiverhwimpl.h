#pragma once

#include "ireceiver.h"
#include "rtl-sdr.h"

#include <memory>
#include <thread>

class ReceiverHWImpl : public IReceiver {
public:
    ReceiverHWImpl( uint32_t numberDev = 0);
    ~ReceiverHWImpl() override;

    virtual void setSettingsReceiver(BaseSettingsReceiver* sett) override final;
    virtual void setSettingsTransaction(BaseSettingTransaction* sett) override final;

    void setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) override final;

    void start() override final;
    void stop() override final;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> m_d;
    SettingTransaction settingTransaction;

    std::unique_ptr<std::thread> thread;

    virtual bool getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) override final;
    void startLoop();
    void startSingle();
};

class RtlSdrDev {
public:
    static uint32_t deviceSearch() {
        char vendor[256] = {0}, product[256] = {0}, serial[256] = {0};
        auto device_count = rtlsdr_get_device_count();
        if(!device_count) {
            std::cerr << "No supported devices found.\n";
            return {};
        }
        std::cerr << "Found " << device_count << " device(s):\n";
        for(uint32_t i = 0; i < device_count; i++) {
            if(rtlsdr_get_device_usb_strings(i, vendor, product, serial) == 0) {
                std::cerr << i << ": " << vendor << ", " << product << ", " << serial << "\n";
            } else {
                std::cerr << i << "Failed to query data\n";
            }
        }
        return device_count;
    }
};