#include <gtest/gtest.h>
#include "dsp/fft.h"
#include <vector>
#include <fstream>
#include "receiver/receiversoftimpl.h"
#include "receiver/receiverwrapper.h"

TEST( fakeReceiverTest, creating ) {

    auto testfakeImpl = ReceiverWrapper::getReceiverByName( "fake" );
    ASSERT_NE(testfakeImpl, nullptr);

}

TEST( fakeReceiverTest, getComplexTest ) {

    std::ifstream file( "bin/checkFakeCom", std::ifstream::binary );
    if( !file.is_open() ) {
        throw std::runtime_error( "cant open file " );
    }

    std::vector< Complex< uint8_t > > vec( 10 );

    for( uint32_t i = 0; i < 10; i++ ) {
        file.read( reinterpret_cast< char* >( &( vec[ i ].re ) ), sizeof( uint8_t ) );
        file.read( reinterpret_cast< char* >( &( vec[ i ].im ) ), sizeof( uint8_t ) );
    }
    file.close();

    auto testfakeImpl = ReceiverWrapper::getReceiverByName( "fake" );
    sinParams sin1 { 6, 10 };

    fakeParams fakeset;
    fakeset.fd = 1600;
    fakeset.sampleCount = 1600;
    fakeset.noiseLVL = -100;

    fakeset.sinPar = { sin1 };

    BaseSettings* fp = &fakeset;
    testfakeImpl->setSettings( fp );

    std::vector< Complex< uint8_t > > Buf2;
    testfakeImpl->getComplex(  Buf2 );

    for( uint64_t i = 0; i < 10; ++i ) {

        ASSERT_NEAR( Buf2[ i ].re, vec[ i ].re, 1 );
        ASSERT_NEAR( Buf2[ i ].im, vec[ i ].im, 1 );

    }
}

TEST( fakeReceiverTest, getSpectrumTest ) {

    std::ifstream file( "bin/checkFakeSpec", std::ifstream::binary );
    if( !file.is_open() ) {
        throw std::runtime_error( "cant open file " );
    }

    std::vector< Complex< double > > vec( 10 );

    for( uint32_t i = 0; i < 10; i++ ) {
        file.read( reinterpret_cast< char* >( &( vec[ i ].re ) ), sizeof( double ) );
        file.read( reinterpret_cast< char* >( &( vec[ i ].im ) ), sizeof( double ) );
    }

    file.close();

    auto testfakeImpl = ReceiverWrapper::getReceiverByName( "fake" );
    sinParams sin1 { 6, 10 };

    fakeParams fakeset;
    fakeset.fd = 3200;
    fakeset.sampleCount = 1600;
    fakeset.noiseLVL = -190; //шум в дБ

    fakeset.sinPar = { sin1 };

    BaseSettings* fp = &fakeset;
    testfakeImpl->setSettings( fp );

    std::vector< Complex< double > > Buf2;
    testfakeImpl->getSpectrum( Buf2 );

    for( uint64_t i = 0; i < 10; ++i ) {

        ASSERT_NEAR( Buf2[ i ].abs(), vec[ i ].abs(), 1e-5 );

    }

}
