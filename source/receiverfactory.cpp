#include "receiver/receiverfactory.h"
#include "receiver/receiverhwimpl.h"
#include "receiver/receiversoftimpl.h"

std::unique_ptr< IReceiver > ReceiverFactory::create( ReceiverParams params ) {
    if( params.receiverType == ReceiverParams::ReceiverType::hw )
        return std::make_unique< ReceiverHWImpl >( params.bufferSize );

    if( params.receiverType == ReceiverParams::ReceiverType::fake )
        return std::make_unique< FakeReceiver >( params.bufferSize );

    return nullptr;
}
