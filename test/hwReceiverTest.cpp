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
    ReceiverFactory::ReceiverParams params{
        ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::single, 0, true}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

    rec->setSettings(&recset);

    size_t iterCount = 3;
    rec->setCallBack([&iterCount, &rec](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 1; i < size; ++i) {
            ASSERT_TRUE(uint8_t(uintPtr[i - 1] + 1) == uintPtr[i]);
        }

        iterCount--;
        if(iterCount == 0) {
            rec->stop();
        }
    });

    rec->start();
}

TEST_F(ReceiverTest, startTestModeHwLoop) {
    size_t irqSize    = 8;
    size_t bufferSize = 1024 * 16 * irqSize;
    ReceiverFactory::ReceiverParams params{
        ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::loop, irqSize, true}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

    rec->setSettings(&recset);

    std::vector<uint8_t> buf(0);
    buf.reserve(1024 * 1024 * 512);
    auto last = std::chrono::steady_clock::now();
    rec->setCallBack([&buf, &last](Complex<int8_t>* ptr, uint32_t size) {
        auto start = std::chrono::steady_clock::now();
        std::cout << "разница по времени между callback "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(start - last).count() << "ms" << std::endl;

        auto uintPtr = (uint8_t*)(ptr);
        buf.insert(buf.end(), uintPtr, uintPtr + size);
        //  std::cerr << " buffer cap: " << buf.capacity() <<std::endl;
        last = std::chrono::steady_clock::now();
    });
    rec->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    for(std::size_t i = 1; i != buf.size(); i++) {
        std::cout << i << "  " << +buf.at(i) << std::endl;
        ASSERT_EQ(uint8_t(buf[i - 1] + 1), uint8_t(buf[i]));
    }
}

TEST_F(ReceiverTest, startTestModeHwLoop_long) {
    size_t irqSize    = 4;
    size_t bufferSize = 1024 * 256;
    ReceiverFactory::ReceiverParams params{
        ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::loop, irqSize, true}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

    rec->setSettings(&recset);

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
    rec->start();
    std::this_thread::sleep_for(std::chrono::minutes(30));
    rec->stop(); // нужно чтобы драйвер успел остановить поток loop перед тем как мы закроем свисток

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(ReceiverTest, startTestModeHwSingle_2_Dev) {
    size_t bufferSize    = 512 * 1024 * 4;
    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

    auto count = RtlSdrDev::deviceSearch();
    ASSERT_TRUE(count == 2);

    auto rec1 = ReceiverFactory::create(
        {ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::single, 0, true}});

    auto rec2 = ReceiverFactory::create(
        {ReceiverFactory::ReceiverParams::ReceiverType::hw, 1, {bufferSize, TypeTransaction::single, 0, true}});

    size_t iterCount1 = 3;
    rec1->setCallBack([&iterCount1, &rec1](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 1; i < size; ++i) {
            ASSERT_TRUE(uint8_t(uintPtr[i - 1] + 1) == uintPtr[i]);
        }

        iterCount1--;
        if(iterCount1 == 0) {
            rec1->stop();
        }
    });

    size_t iterCount2 = 3;
    rec2->setCallBack([&iterCount2, &rec2](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        for(uint32_t i = 1; i < size; ++i) {
            ASSERT_TRUE(uint8_t(uintPtr[i - 1] + 1) == uintPtr[i]);
        }

        iterCount2--;
        if(iterCount2 == 0) {
            rec2->stop();
        }
    });

    std::thread t1([&rec1, &recset]() {
        rec1->setSettings(&recset);
        rec1->start();
    });
    std::thread t2([&rec2, &recset]() {
        rec2->setSettings(&recset);
        rec2->start();
    });
    t1.join();
    t2.join();
}

TEST_F(ReceiverTest, startTestModeHwLoop_2_Dev) {
    size_t ircSize       = 4;
    size_t bufferSize    = 256 * 1024 * ircSize;
    uint32_t centralFreq = 0;
    uint32_t sampleFreq  = 0;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

    auto count = RtlSdrDev::deviceSearch();
    ASSERT_TRUE(count == 2);

    auto rec1 = ReceiverFactory::create(
        {ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::loop, ircSize, true}});

    auto rec2 = ReceiverFactory::create(
        {ReceiverFactory::ReceiverParams::ReceiverType::hw, 1, {bufferSize, TypeTransaction::loop, ircSize, true}});

    std::vector<uint8_t> buf1(0);
    buf1.reserve(1024 * 1024 * 512);
    rec1->setCallBack([&buf1](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf1.insert(buf1.end(), uintPtr, uintPtr + size);
    });

    std::vector<uint8_t> buf2(0);
    buf2.reserve(1024 * 1024 * 512);
    rec2->setCallBack([&buf1](Complex<int8_t>* ptr, uint32_t size) {
        auto uintPtr = (uint8_t*)(ptr);
        buf1.insert(buf1.end(), uintPtr, uintPtr + size);
    });
    rec1->setSettings(&recset);
    rec2->setSettings(&recset);

    rec1->start();
    rec2->start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    rec1->stop();
    rec2->stop();
}
TEST_F(ReceiverTest, startHwLoop) {
    size_t irqSize    = 15;
    size_t bufferSize = 1024 * 256 * irqSize;
    ReceiverFactory::ReceiverParams params{
        ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::loop, irqSize}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 200e6;
    uint32_t sampleFreq  = 1e6;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);
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
    ReceiverFactory::ReceiverParams params{
        ReceiverFactory::ReceiverParams::ReceiverType::hw, 0, {bufferSize, TypeTransaction::single, 0}};
    auto rec = ReceiverFactory::create(params);

    uint32_t centralFreq = 170e6;
    uint32_t sampleFreq  = 1.2e6;
    int32_t gain         = 0;
    RfSettings sett{centralFreq, sampleFreq, gain};

    ReceiverSettings recset(sett);

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
