#include "receiver/receiversoftimpl.h"

FakeReceiver::FakeReceiver() { }

FakeReceiver::~FakeReceiver() {
    // delete f_p;
}

void FakeReceiver::setSettingsReceiver(BaseSettingsReceiver* settings) {
    this->f_p = dynamic_cast<fakeParams*>(settings);
    complexBuff.resize(f_p->sampleCount);
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
    needProcessing = true;
    while(isNeedProcessing()) {
        getComplex(complexBuff.data(), f_p->sampleCount);
        process(complexBuff.data(), f_p->sampleCount);
    }
}

void FakeReceiver::setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> f) {
    process = f;
}
void FakeReceiver::stop() {
    needProcessing = false;
}
