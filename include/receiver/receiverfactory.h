#pragma once

#include "ireceiver.h"
#include <memory>

class ReceiverFactory {
public:
    struct ReceiverParams {
        enum class ReceiverType {
            fake, hw
        }
        receiverType;

        size_t bufferSize{ };
    };

    static std::unique_ptr< IReceiver > create( ReceiverParams params );
};

