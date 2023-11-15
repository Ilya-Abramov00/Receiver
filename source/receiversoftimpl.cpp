#include "receiver/receiversoftimpl.h"

FakeReceiver::FakeReceiver() {

}

FakeReceiver::~FakeReceiver() {


    // delete f_p;
}




void FakeReceiver::setSettings( BaseSettings* settings ) {

    fakeParams* fakeset =  dynamic_cast<  fakeParams* >( settings );
    this->f_p = fakeset;
}


template < typename Type >
std::vector< Complex< Type > > FakeReceiver::GenSignal( const fakeParams* fakeset ) {
    GenNoise WNGen;
    Generator_sin SnGen;
    std::vector< Complex< double > > Wdata = WNGen.GenWN< double >( fakeset->noiseLVL, fakeset->sampleCount );

    double a0 =  sqrt( 1.5 ) * pow( 10, fakeset->noiseLVL / 20 );// sqrt( pow( 10, fakeset->noiseLVL / 20 ) )
    Complex< double > A0;
    A0.re = a0;
    A0.im = A0.re;
    std::vector< Complex< Type > > data( fakeset->sampleCount );

    for( uint32_t i = 0; i < fakeset->sinPar.size(); i++ ) {

        std::vector< Complex< double > > dataSin = SnGen.gen_sin< double >( fakeset->sinPar[ i ].amp,  fakeset->sinPar[ i ].freq, fakeset->fd,  fakeset->sampleCount );
        Complex< double > a = 0;
        a.im = fakeset->sinPar[ i ].amp;
        // a.re = a.im;
        for( uint64_t j = 0; j < fakeset->sampleCount; j++ ) {
            Wdata[ j ] += dataSin[ j ] + a;
        }
    }
    for( uint64_t j = 0; j < fakeset->sampleCount; j++ ) {
        data[ j ] += Wdata[ j ] + A0;
    }
    return data;
}


bool FakeReceiver::getComplex( const BaseSettings* settings, Buffer& out ) {

    const fakeParams* fakeset =  dynamic_cast< const fakeParams* >( settings );
    auto sizeBuff = fakeset->sampleCount;
    out.resize( sizeBuff );
    std::vector< Complex< uint8_t > > data = this->GenSignal< uint8_t >( fakeset );
    for( uint64_t i = 0; i < fakeset->sampleCount; i++ ) {
        out[ i ] = data[ i ];
    }
    return 1;
}

void FakeReceiver::getSpectrum( const BaseSettings* settings, SpectBuff& out ) {

    const fakeParams* fakeset =  dynamic_cast< const fakeParams* >( settings );
    SpectBuff dataIN = this->GenSignal< double >( fakeset );
    auto sizeBuff = fakeset->sampleCount;
    out.resize( sizeBuff );
    fft( dataIN, out, fakeset->sampleCount );


}

bool FakeReceiver::getComplex(  Buffer& out ) {

    std::vector< Complex< uint8_t > > data = this->GenSignal< uint8_t >( f_p );
    auto sizeBuff = f_p->sampleCount;
    out.resize( sizeBuff );
    for( uint64_t i = 0; i < f_p->sampleCount; i++ ) {
        out[ i ] = data[ i ];
    }
    return 1;

}
void FakeReceiver::getSpectrum(  SpectBuff& out ) {


    SpectBuff dataIN = this->GenSignal< double >(  f_p );
    auto sizeBuff =  f_p->sampleCount;
    out.resize( sizeBuff );
    fft( dataIN, out,  f_p->sampleCount );

}

bool FakeReceiver::getComplex( Complex< uint8_t >* complexBuff, uint32_t sizeOfBuff ) {

    Complex a( 0, 1 );

    for( uint32_t i = 0; i != sizeOfBuff; i++ ) {

        complexBuff[ i ] = a;
    }

    return true;
}

void FakeReceiver::start() {

    uint32_t counter = complexBuff.size();
    uint32_t readSize = 256;
    while( true ) {

        getComplex( complexBuff.data(), readSize );

        counter -= readSize;

        if( counter == 0 ) {
            process( complexBuff.data(), complexBuff.size() );
            complexBuff.clear();
            counter = complexBuff.size();
        }
    }

}

void FakeReceiver::setCallBack( std::function< void( Complex< uint8_t >*, uint32_t ) > f ) {
    process = f;
}




