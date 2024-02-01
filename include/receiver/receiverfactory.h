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

        SettingTransaction settingTransaction;
    };

    static std::unique_ptr< IReceiver > create( ReceiverParams params );
};
