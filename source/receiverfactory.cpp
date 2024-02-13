#include "receiver/common/receiverfactory.h"

#include "receiver/impl/receiverhwimpl.h"
#include "receiver/fake/receiversoftimpl.h"

std::unique_ptr<IReceiver> ReceiverFactory::create(ReceiverParams params) {
    switch(params.receiverType) {
        case(ReceiverParams::ReceiverType::hw):
            return std::make_unique<ReceiverHWImpl>(params.numberDev);
        case(ReceiverParams::ReceiverType::fake):
            return std::make_unique<FakeReceiver>();
        default:
            return nullptr;
    }
}
