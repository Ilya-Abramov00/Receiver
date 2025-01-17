#include "receiver/common/receiverfactory.h"
#include "receiver/impl/receiverhwimpl.h"

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

class ReceiverTestModeTest : public ::testing::Test {
protected:
    void SetUp() { }
    void TearDown() { }
};

TEST_F(ReceiverTestModeTest, startTestModeHwSingle) {
    size_t bufferSize = 1024 * 512;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw, 0};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 1e6;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::single, 0, true);

    rec->setSettingsReceiver(&set);
    rec->setSettingsTransaction(&setTransaction);

    std::size_t iter = 15;

    size_t iterCount = iter;
    rec->setCallBack([&iterCount, &rec](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 1; i < size; ++i) {
            // std::cout << i << "  " << +uintPtr[i] << std::endl;
            ASSERT_EQ(uint8_t(uintPtr[i - 1] + 1), uintPtr[i]);
        }

        iterCount--;
        if(iterCount == 0) {
            rec->stop();
        }
    });

    rec->start();

    SettingTransaction setTransaction1(bufferSize / 16, TypeTransaction::single, 0, true);
    rec->setSettingsTransaction(&setTransaction);
    iterCount = iter;
    rec->setCallBack([&iterCount, &rec](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 1; i < size; ++i) {
            // std::cout << i << "  " << +uintPtr[i] << std::endl;
            ASSERT_EQ(uint8_t(uintPtr[i - 1] + 1), uintPtr[i]);
        }

        iterCount--;
        if(iterCount == 0) {
            rec->stop();
        }
    });
}

TEST_F(ReceiverTestModeTest, startTestModeHwLoop) {
    size_t irqSize    = 8;
    size_t bufferSize = 1024 * 128;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw, 0};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 1.4e6;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::loop, irqSize, true);

    rec->setSettingsReceiver(&set);
    rec->setSettingsTransaction(&setTransaction);

    std::vector<uint8_t> buf(0);
    buf.reserve(1024 * 1024 * 512);
    auto last = std::chrono::steady_clock::now();
    rec->setCallBack([&buf, &last](Complex<int8_t>* ptr, uint32_t size) {
        auto start = std::chrono::steady_clock::now();
        std::cout << "разница по времени между callback "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(start - last).count() << "ms" << std::endl;

        auto uintPtr = (uint8_t*)(ptr);
        buf.insert(buf.end(), uintPtr, uintPtr + size);
        last = std::chrono::steady_clock::now();
    });
    rec->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(4500));
    rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    for(std::size_t i = 1; i != buf.size(); i++) {
        ASSERT_EQ(uint8_t(buf[i - 1] + 1), uint8_t(buf[i]));
    }
}

TEST_F(ReceiverTestModeTest, startTestModeHwSingle_2_Dev) {
    size_t bufferSize    = 1024 * 4;
    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 300e3;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::single, 0, true);

    auto count = RtlSdrDev::deviceSearch();
    ASSERT_TRUE(count == 2);

    auto rec1 = ReceiverFactory::create({ReceiverFactory::ReceiverParams::ReceiverType::hw, 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    auto rec2 = ReceiverFactory::create({ReceiverFactory::ReceiverParams::ReceiverType::hw, 1});

    std::vector<uint8_t> buf1(0, bufferSize);
    size_t iterCount1 = 3;
    rec1->setCallBack([&rec1, &buf1, &iterCount1](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf1.insert(buf1.end(), uintPtr, uintPtr + size);
        iterCount1--;
        if(iterCount1 == 0) {
            rec1->stop();
        }
    });

    std::vector<uint8_t> buf2(0, bufferSize);
    size_t iterCount2 = 3;
    rec2->setCallBack([&rec2, &buf2, &iterCount2](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf2.insert(buf2.end(), uintPtr, uintPtr + size);

        iterCount2--;
        if(iterCount2 == 0) {
            rec2->stop();
        }
    });

    rec1->setSettingsReceiver(&set);
    rec1->setSettingsTransaction(&setTransaction);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    rec2->setSettingsReceiver(&set);
    rec2->setSettingsTransaction(&setTransaction);

    std::thread t1([&rec1]() { rec1->start(); });
    std::thread t2([&rec2]() { rec2->start(); });
    t1.join();
    t2.join();
    for(std::size_t i = 1; i < buf1.size(); i++) {
        // std::cout << i << "  " << +buf1[i] << std::endl;
        ASSERT_EQ(uint8_t(buf1[i - 1] + 1), uint8_t(buf1[i]));
    }
    for(std::size_t i = 1; i < buf2.size(); i++) {
        // std::cout << i << "  " << +buf2[i] << std::endl;
        ASSERT_EQ(uint8_t(buf2[i - 1] + 1), uint8_t(buf2[i]));
    }
}

TEST_F(ReceiverTestModeTest, startTestModeHwLoop_2_Dev) {
    size_t ircSize       = 4;
    size_t bufferSize    = 1024 * 32;
    uint32_t centralFreq = 10e6;
    uint32_t sampleFreq  = 2e6;

    ReceiverSettings set{centralFreq, sampleFreq};
    SettingTransaction setTransaction(bufferSize, TypeTransaction::loop, ircSize, true);

    auto count = RtlSdrDev::deviceSearch();
    ASSERT_TRUE(count == 2);

    auto rec1 = ReceiverFactory::create({ReceiverFactory::ReceiverParams::ReceiverType::hw, 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    auto rec2 = ReceiverFactory::create({ReceiverFactory::ReceiverParams::ReceiverType::hw, 1});

    std::vector<uint8_t> buf1(0);
    buf1.reserve(1024 * 1024 * 512);
    rec1->setCallBack([&buf1](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf1.insert(buf1.end(), uintPtr, uintPtr + size);
    });

    std::vector<uint8_t> buf2(0);
    buf2.reserve(1024 * 1024 * 512);
    rec2->setCallBack([&buf2](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf2.insert(buf2.end(), uintPtr, uintPtr + size);
    });

    rec1->setSettingsReceiver(&set);
    rec1->setSettingsTransaction(&setTransaction);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    rec2->setSettingsReceiver(&set);
    rec2->setSettingsTransaction(&setTransaction);

    rec1->start();
    rec2->start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    rec1->stop();
    rec2->stop();

    for(std::size_t i = 1; i != buf1.size(); i++) {
        ASSERT_EQ(uint8_t(buf1[i - 1] + 1), uint8_t(buf1[i]));
    }
    for(std::size_t i = 1; i != buf2.size(); i++) {
        //  std::cout << i << "  " << +buf2[i] << std::endl;
        ASSERT_EQ(uint8_t(buf2[i - 1] + 1), uint8_t(buf2[i]));
    }
}

TEST_F(ReceiverTestModeTest, startTestModeHwLoop_freq) {
    size_t bufferSize = 1024 * 2;
    bool flag         = true;
    for(std::size_t irqSize = 1; irqSize != 16; irqSize++) {
        for(std::size_t freq = 1e6; flag == true; freq += 50e3) {
            {
                ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::hw, 0};
                auto rec = ReceiverFactory::create(params);

                uint32_t centralFreq = 200e6;
                uint32_t sampleFreq  = freq;

                ReceiverSettings set{centralFreq, sampleFreq};
                SettingTransaction setTransaction(bufferSize, TypeTransaction::loop, irqSize, true);

                rec->setSettingsReceiver(&set);
                rec->setSettingsTransaction(&setTransaction);

                std::vector<uint8_t> buf(0);
                buf.reserve(1024 * 1024 * 512);
                rec->setCallBack([&buf](Complex<int8_t>* ptr, uint32_t size) {
                    std::cerr << "SIZe= " << size << std::endl;
                    auto uintPtr = (uint8_t*)(ptr);
                    buf.insert(buf.end(), uintPtr, uintPtr + size);
                });
                rec->start();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                for(std::size_t i = 1; i != buf.size(); i++) {
                    if(uint8_t(buf[i - 1] + 1) != uint8_t(buf[i])) {
                        flag = false;
                    }
                }
                if(!flag) {
                    std::cerr << "FAIL " << std::endl;
                    std::cerr << "freq = " << freq / 1e3 << std::endl;
                    std::cerr << "irqsize = " << irqSize << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        flag = true;
    }
}
