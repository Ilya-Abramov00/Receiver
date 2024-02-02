#include "dsp/fft.h"
#include "receiver/receiverfactory.h"
#include "receiver/receiversoftimpl.h"

#include <fstream>
#include <gtest/gtest.h>
#include <vector>

TEST(fakeReceiverTest, creating) {
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::fake, {}};
    auto testfakeImpl = ReceiverFactory::create(params);
    ASSERT_NE(testfakeImpl, nullptr);
}

TEST(fakeReceiverTest, getComplexTest) {
    size_t bufferSize = 3000;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::fake,0,
                                           {bufferSize, TypeTransaction::loop}};
    auto testfakeImpl = ReceiverFactory::create(params);
    sinParams sin1{100, 10};

    fakeParams fakeset;
    fakeset.fd          = 3000;
    fakeset.sampleCount = 3000;
    fakeset.noiseLVL    = 10;

    fakeset.sinPar = {sin1};

    BaseSettings* fp = &fakeset;
    testfakeImpl->setSettings(fp);

    std::vector<Complex<uint8_t> > Buf2;
    testfakeImpl->getComplex(Buf2);

    std::ofstream out("out_getComplexTest.iqb");

    out.write(reinterpret_cast<char*>(Buf2.data()), Buf2.size() * 2 * sizeof(uint8_t));
    out.close();
}

TEST(fakeReceiverTest, startTestFake) {
    size_t sampleCount = 6000;
    ReceiverFactory::ReceiverParams params{ReceiverFactory::ReceiverParams::ReceiverType::fake,0,
                                           {sampleCount, TypeTransaction::loop}};
    auto fakeImpl = ReceiverFactory::create(params);

    sinParams sin1{100, 100};

    fakeParams fakeset;
    fakeset.fd       = 6000;
    fakeset.noiseLVL = -1000;
    fakeset.sinPar   = {sin1};

    fakeImpl->setSettings(&fakeset);

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
