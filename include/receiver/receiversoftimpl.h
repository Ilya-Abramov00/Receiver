#pragma once

#include "ireceiver.h"
#include "gen_noise.h"
#include "gen_sin.h"
#include <memory>
#include "dsp/fft.h"


class FakeReceiver: public IReceiver {
public:

    FakeReceiver();
    ~FakeReceiver();

    virtual void setSettings(  BaseSettings* settings )  override final;
    virtual bool getComplex( const BaseSettings* settings, Buffer& out ) override final;
    virtual void getSpectrum( const BaseSettings* settings, SpectBuff& out )override final;

    virtual bool getComplex(  Buffer& out ) override final;
    virtual void getSpectrum(  SpectBuff& out ) override final;



private:

    void set( const fakeParams* fakeset )const;
    template < typename Type >
    std::vector< Complex< Type > > GenSignal( const fakeParams* fakeset );
    fakeParams* f_p { 0 };


};
