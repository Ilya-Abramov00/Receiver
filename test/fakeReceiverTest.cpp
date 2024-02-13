
#include "dsp/fft.h"
#include "receiver/common/receiverfactory.h"

#include <fstream>
#include <gtest/gtest.h>
#include <vector>

TEST(fakeReceiverTest, creating) {
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::fake, {}};
    auto testfakeImpl = ReceiverFactory::create(params);
    ASSERT_NE(testfakeImpl, nullptr);
}


TEST(fakeReceiverTest, startTestFake) {
    size_t sampleCount = 6000;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::fake,0};
    auto fakeImpl = ReceiverFactory::create(params);

    sinParams sin1{100, 100};

    fakeParams fakeset;
    fakeset.fd       = 6000;
    fakeset.noiseLVL = -1000;
    fakeset.sinPar   = {sin1};
    fakeset.sampleCount   = sampleCount;
    fakeImpl->setSettingsReceiver(&fakeset);

    std::ofstream complexOut("comsig.iqc", std::fstream::binary);

    int iter = 1;

    fakeImpl->setCallBack([&complexOut, &iter, &fakeImpl](Complex<int8_t>* data, uint32_t dataSize) {
        --iter;
        complexOut.write((char*)(data), sizeof(uint8_t) * 2 * dataSize);
        complexOut.close();

        if(iter == 0) {
            fakeImpl->stop();
        }
    });

    fakeImpl->start();
}

