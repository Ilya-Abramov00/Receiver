#include "receiver/receiversoftimpl.h"

FakeReceiver::FakeReceiver( size_t bufferSize ) : IReceiver( bufferSize ) {

}

FakeReceiver::~FakeReceiver() {


    // delete f_p;
}




void FakeReceiver::setSettings( BaseSettings* settings ) {

    fakeParams* fakeset =  dynamic_cast<  fakeParams* >( settings );
    this->f_p = fakeset;
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

    std::vector< Complex< int8_t > > data = this->GenSignal< int8_t >( f_p );
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

bool FakeReceiver::getComplex( Complex< int8_t >* complexBuff, uint32_t sizeOfBuff ) {

    f_p->sampleCount = sizeOfBuff;
    std::vector< Complex< int8_t > > data = GenSignal< int8_t >( f_p );
    for( uint32_t i = 0; i != sizeOfBuff; i++ ) {
        *( complexBuff + i ) = data[ i ];
    }

    return true;

}

void FakeReceiver::start() {

    needProcessing = true;
    size_t counter = bufferSize;
    uint32_t readSize = bufferSize / 2;
    while( isNeedProcessing() ) {

        getComplex( complexBuff.data(), readSize );

        counter -= readSize;

        if( counter == 0 ) {
            process( complexBuff.data(), bufferSize );
            counter = bufferSize;
        }
    }

}

void FakeReceiver::setCallBack( std::function< void( Complex< int8_t >*, uint32_t ) > f ) {
    process = f;
}




