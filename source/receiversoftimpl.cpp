#include "receiver/receiversoftimpl.h"

FakeReceiver::FakeReceiver()  { }

FakeReceiver::~FakeReceiver() {
    // delete f_p;
}

void FakeReceiver::setSettingsReceiver(BaseSettingsReceiver* settings) {
    this->f_p = dynamic_cast<fakeParams*>(settings);
}


bool FakeReceiver::getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) {
    f_p->sampleCount                   = sizeOfBuff;
    std::vector<Complex<int8_t> > data = GenSignal<int8_t>(f_p);
    for(uint32_t i = 0; i != sizeOfBuff; i++) {
        *(complexBuff + i) = data[i];
    }

    return true;
}

void FakeReceiver::start() {
/*    needProcessing    = true;
    size_t counter    = settingTransaction.bufferSize;
    uint32_t readSize = settingTransaction.bufferSize / 2;
    while(isNeedProcessing()) {
        getComplex(complexBuff.data(), readSize);

        counter -= readSize;

        if(counter == 0) {
            process(complexBuff.data(), settingTransaction.bufferSize);
            counter = settingTransaction.bufferSize;
        }
    }*/
}

void FakeReceiver::setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) {
    process = f;
}
void FakeReceiver::stop() {
    needProcessing = false;
}
