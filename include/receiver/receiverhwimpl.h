#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include "ireceiver.h"

class ReceiverHWImpl: public IReceiver {
public:
    ReceiverHWImpl( size_t bufferSize );
    ~ReceiverHWImpl() override;

    virtual bool getComplex( const BaseSettings* settings, Buffer& out ) override final;
    virtual void getSpectrum( const BaseSettings* settings, SpectBuff& out )override final;
    virtual void setSettings(  BaseSettings* sett )  override final;

    virtual void start() override final;

    virtual bool getComplex(  Buffer& out  ) override final;
    virtual void getSpectrum(  SpectBuff& out ) override final;

    void setCallBack(std::function<void (Complex<int8_t> *, uint32_t)> f ) override final;

private:
    struct Pimpl;
    std::unique_ptr< Pimpl > m_d;
    virtual bool getComplex(Complex<int8_t> *complexBuff, uint32_t sizeOfBuff ) override final;

};


