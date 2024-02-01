#include "receiver/receiverfactory.h"
#include "receiver/receiverhwimpl.h"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

class ReceiverTest : public ::testing::Test {
protected:
    void SetUp() { }
    void TearDown() { }
};

TEST_F(ReceiverTest, startTestModeHwSingle) {
    size_t bufferSize = 512;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw,
                                           {bufferSize, TypeTransaction::single, 0}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset;
    recset.rfSettings = sett;

    rec->setSettings(&recset);
    rec->setTestMode();

    size_t iterCount = 3;
    rec->setCallBack([&iterCount, &rec](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);

        std::cout << "iterCount: " << iterCount << "  size =" << size << std::endl;
        for(uint32_t i = 0; i < size; ++i) {
            std::cout << "num: " << i << ", " << +uintPtr[i] << std::endl;
            ASSERT_TRUE(uint8_t(uintPtr[i] + 1) == uintPtr[i + 1]);
        }

        iterCount--;
        if(iterCount == 0) {
            rec->stop();
        }
    });

    rec->start();
}

TEST_F(ReceiverTest, startTestModeHwLoop) {
    size_t irqSize    = 4;
    size_t bufferSize = 512 * irqSize;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw,
                                           {bufferSize, TypeTransaction::loop, irqSize}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset;
    recset.rfSettings = sett;

    rec->setSettings(&recset);
    rec->setTestMode();

    uint8_t last;
    bool begin       = true;
    std::size_t iter = 0;
    rec->setCallBack([&last, &begin, &iter](Complex<int8_t>* ptr, uint32_t size) {
        iter++;
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 0; i != size; ++i) {
            if(i == 0) {
                if(begin) {
                    ASSERT_TRUE(uint8_t(uintPtr[i] + 1) == uintPtr[i + 1]);
                    begin = false;
                } else {
                    ASSERT_TRUE(uint8_t(last + 1) == uintPtr[i]);
                    ASSERT_TRUE(uint8_t(uintPtr[i] + 1) == uintPtr[i + 1]);
                }
            } else if(i < size - 1) {
                ASSERT_TRUE(uint8_t(uintPtr[i] + 1) == uintPtr[i + 1]);
            } else if(i == size - 1) {
                last = uintPtr[i];
            }
        }
    });
    std::cout << "Получено " << iter << " callback";
    rec->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(ReceiverTest, startHwLoop) {
    size_t irqSize    = 4;
    size_t bufferSize = 1024 * 4 * irqSize;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw,
                                           {bufferSize, TypeTransaction::loop, irqSize}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 150e6;
    uint32_t sampleFreq  = 1e6;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset;
    recset.rfSettings = sett;
    rec->setSettings(&recset);

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
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw,
                                           {bufferSize, TypeTransaction::single, 0}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 170e6;
    uint32_t sampleFreq  = 1.2e6;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset;
    recset.rfSettings = sett;

    rec->setSettings(&recset);

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
