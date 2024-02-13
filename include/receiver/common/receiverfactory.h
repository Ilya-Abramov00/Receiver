#pragma once

#include "ireceiver.h"

#include <memory>

class ReceiverFactory {
public:
    struct ReceiverParams {
        enum class ReceiverType { fake, hw } receiverType;
        uint32_t numberDev{0};
    };

    static std::unique_ptr<IReceiver> create(ReceiverParams params);
};
