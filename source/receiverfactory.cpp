#include "receiver/receiverfactory.h"

#include "receiver/receiverhwimpl.h"
#include "receiver/receiversoftimpl.h"

std::unique_ptr<IReceiver> ReceiverFactory::create(ReceiverParams params) {
    switch(params.receiverType) {
        case(ReceiverParams::ReceiverType::hw):
            return std::make_unique<ReceiverHWImpl>(params.settingTransaction,params.numberDev);
        case(ReceiverParams::ReceiverType::fake):
            return std::make_unique<FakeReceiver>(params.settingTransaction);
        default:
            return nullptr;
    }
}
