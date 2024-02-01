#pragma once

#include "ireceiver.h"
#include "gen_noise.h"
#include "gen_sin.h"
#include <memory>
#include "dsp/fft.h"

class FakeReceiver: public IReceiver {
public:

    FakeReceiver( SettingTransaction settingTransaction );
    ~FakeReceiver();

    virtual void setSettings(  BaseSettings* settings )  override final;
    virtual bool getComplex( const BaseSettings* settings, Buffer& out ) override final;
    virtual void getSpectrum( const BaseSettings* settings, SpectBuff& out )override final;

    virtual bool getComplex(  Buffer& out ) override final;
    virtual void getSpectrum(  SpectBuff& out ) override final;

    virtual void start() override final;
    virtual void stop() override final;
    void setCallBack( std::function< void( Complex< int8_t >*, uint32_t ) > f ) override final;

private:

    void set( const fakeParams* fakeset )const;
    template < typename Type >
    std::vector< Complex< Type > > GenSignal( const fakeParams* fakeset );
    fakeParams* f_p { 0 };

    virtual bool getComplex( Complex< int8_t >* complexBuff, uint32_t sizeOfBuff ) override final;

    template < typename T >
    std::vector< Complex< T > > normData( std::vector< Complex< float > >& dataIn ) {
        auto ptrData    = reinterpret_cast< const float* >( dataIn.data() );
        auto maxElement = std::max_element( ptrData, ptrData + dataIn.size() * 2, [] ( const float& a, const float& b ) {
            return std::abs( a ) < std::abs( b );
        } );

        auto scale = static_cast< float >( std::numeric_limits< T >::max() ) / std::abs( *maxElement );


        std::vector< Complex< T > > dataOut( dataIn.size() );
        for( size_t i = 0; i < dataIn.size(); ++i ) {
            dataOut[ i ] = { static_cast< T >(  dataIn[ i ].re  * scale ), static_cast< T >( dataIn[ i ].im  * scale ) };
        }

        return dataOut;
    }

};

template < typename Type >
std::vector< Complex< Type > > FakeReceiver::GenSignal( const fakeParams* fakeset ) {
    GenNoise noiseGen;
    GeneratorSin sinGen;
    std::vector< Complex< float > > data( fakeset->sampleCount );

    for( uint32_t i = 0; i < fakeset->sinPar.size(); i++ ) {

        data = sinGen.genSin( fakeset->sinPar[ i ].amp,  fakeset->sinPar[ i ].freq, fakeset->fd,  fakeset->sampleCount );

    }
    auto noiseData = noiseGen.GenWN< float >( fakeset->noiseLVL, fakeset->sampleCount );
    for( size_t i = 0; i < data.size(); ++i ) {
        data[ i ] += noiseData[ i ];
    }

    auto dataOut = normData< Type >( data );

    return dataOut;
}
