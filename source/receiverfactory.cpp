#include "receiver/receiverfactory.h"
#include "receiver/receiverhwimpl.h"
#include "receiver/receiversoftimpl.h"

std::unique_ptr< IReceiver > ReceiverFactory::getReceiverByName( std::string name ) {
    if( name == "hw" )
        return std::make_unique< ReceiverHWImpl >();

    if( name == "fake" )
        return std::make_unique< FakeReceiver >();

    return nullptr;
}
