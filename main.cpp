#include <iostream>
#include "receiver/receiverwrapper.h"

#include <vector>

using namespace std;

// AGC mode changed to on
// fake: complex and spectrum writing in file and have 2nd version (withtout settings)
// real: complex and spectrum writing in file and have 2nd version (withtout settings)
// real: working correct, write complex on Fc=0
// fake: working correct, but GenW is uncorrect


int main() {
//real receiver:
 auto reImpl = ReceiverWrapper::getReceiverByName( "hw" );
 uint32_t centralFreq = 88900000;
 uint32_t sampleFreq = 960000;
 RfSettings sett { centralFreq, sampleFreq, 490 };

 ReceiverSettings recset;
 recset.sampleCount = ( uint32_t )4194304; // must be 512 or higher 4194304
 recset.rfSettings = sett;
 BaseSettings* rp = &recset;
 reImpl->setSettings( rp );

 std::vector< Complex< uint8_t > > Buf;

 std::vector< Complex< double > > SpBuf;
 reImpl->getSpectrum( rp, SpBuf );

 std::ofstream complexOut( "/home/anatoly/work/workC++/Demodulation/Dest/comsig11.iqc", std::fstream::binary );
 std::ofstream spectOut( "/home/anatoly/work/workC++/Demodulation/Dest/comsig21.iqc", std::fstream::binary );

 for( uint32_t i = 0; i < 4; i++ ) {
 reImpl->getComplex( Buf );
 uint32_t N = Buf.size();
 complexOut.write( ( char* )( Buf.data() ), sizeof( uint8_t ) * 2 * N );

 }
 complexOut.close();

 for( uint32_t i = 0; i < 4; i++ ) {
 reImpl->getSpectrum( SpBuf );
 uint32_t N = SpBuf.size();
 spectOut.write( ( char* )( SpBuf.data() ), sizeof( double ) * 2 * N );

 }
 spectOut.close();

//fakeReceiver:
/*
    auto fakeImpl = ReceiverWrapper::getReceiverByName( "fake" );

    sinParams sin1 { 6, 10 };
    sinParams sin2 { 0, 30 };
    sinParams sin3 { 0, 30 };

    fakeParams fakeset;
    fakeset.fd = 1600;
    fakeset.sampleCount = 1600;
    fakeset.noiseLVL = 18;

    fakeset.sinPar = { sin1, sin2, sin3 };

    BaseSettings* fp = &fakeset;
    fakeImpl->setSettings( fp );

    std::vector< Complex< uint8_t > > Buf2;
    fakeImpl->getComplex(  Buf2 );

    std::vector< Complex< double > > SpecBuf;
    fakeImpl->getSpectrum(  SpecBuf );

    uint32_t N2 = Buf2.size();
    uint32_t SpN2 = SpecBuf.size();

    std::ofstream ComfileOut( "/home/anatoly/work/workC++/Demodulation/Dest/comsig1.iqc", std::fstream::binary );
    ComfileOut.write( ( char* )( Buf2.data() ), sizeof( uint8_t ) * 2 * N2 );
    ComfileOut.close();

    std::ofstream SpecfileOut( "/home/anatoly/work/workC++/Demodulation/Dest/comsig2.iqc", std::fstream::binary );
    SpecfileOut.write( ( char* )( SpecBuf.data() ), sizeof( double ) * 2 * SpN2 );
    SpecfileOut.close();
*/
    return 0;
}

