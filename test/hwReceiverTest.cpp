#include "receiver/common/receiverfactory.h"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

class ReceiverTest : public ::testing::Test {
protected:
    void SetUp() { }
    void TearDown() { }
};

TEST_F(ReceiverTest, startHwLoop) {
    size_t irqSize    = 15;
    size_t bufferSize = 1024 * 256 * irqSize;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw, 0};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 1e6;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::loop, irqSize, false);

    rec->setSettingsReceiver(&set);
    rec->setSettingsTransaction(&setTransaction);

    std::ofstream out("complexdataloop.txt", std::fstream::out | std::fstream::trunc);
    if(!out.is_open()) {
        throw "File could not be opened";
    }
    rec->setCallBack([&out](Complex<int8_t>* ptr, uint32_t size) { out.write((char*)ptr, size); });

    rec->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(ReceiverTest, startHwSingle) {
    size_t bufferSize = 512 * 256;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw, 0};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 1e6;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::single, 0, false);

    rec->setSettingsReceiver(&set);
    rec->setSettingsTransaction(&setTransaction);


    std::ofstream out("complexdatasingle.txt", std::fstream::out | std::fstream::trunc);
    if(!out.is_open()) {
        throw "File could not be opened";
    }
    rec->setCallBack([&out, &rec](Complex<int8_t>* ptr, uint32_t size) {
        out.write((char*)ptr, size);
        rec->stop();
    });

    rec->start();
}
