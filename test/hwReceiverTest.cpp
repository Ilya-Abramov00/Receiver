#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include "receiver/receiverfactory.h"
#include "receiver/receiverhwimpl.h"

TEST( receivers_test, DISABLED_startTestHw ) {

    size_t bufferSize = 1024;
    ReceiverFactory::ReceiverParams params{ ReceiverFactory::ReceiverParams::ReceiverType::hw, bufferSize };
    auto rec = ReceiverFactory::create( params );

    uint32_t centralFreq = 88900000;
    uint32_t sampleFreq = 1000000;
    int32_t gain = 490;
    RfSettings sett { centralFreq, sampleFreq, gain };

    ReceiverSettings recset;
    recset.rfSettings = sett;

    rec->setSettings( &recset );

    float time = 1;
    float sampleCount = time * sampleFreq;
    size_t iterCount = std::ceil( sampleCount / bufferSize );

    std::ofstream out( "out_data.iqb", std::ios::app );
    rec->setCallBack( [ &iterCount, &rec, &out ] ( Complex< int8_t >* ptr, uint32_t size ) {

        for( uint32_t i = 0; i < size; ++i ) {

            out.write( reinterpret_cast< char* >( ptr ), 2 );
            ptr++;
        }

        iterCount--;
        if( iterCount == 0 ) {
            rec->stop();
        }
    } );

    rec->start();

}
