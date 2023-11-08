#pragma once

#include "ireceiver.h"
#include <memory>

class ReceiverFactory
{
public:
    static std::unique_ptr<IReceiver> getReceiverByName(std::string name);
};

