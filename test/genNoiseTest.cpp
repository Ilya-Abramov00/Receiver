#include <gtest/gtest.h>
#include "dsp/fft.h"
#include "receiver/fake/gen_noise.h"
#include <vector>


TEST( TestNoise, WNFine ) {
	Fftw fftw;
    GenNoise noiseGen;
    float w = 10;
    uint64_t n = 4096 * 16;

    std::vector< Complex< float > > data_ = noiseGen.GenWN< float >( w, n );

    fftw.Forward< float >( data_.data(), data_.data(), data_.size() );

	float avr = 0;
    for( uint64_t i = 0; i < n; i++ ) {
		avr += ( data_[ i ].abs() * data_[ i ].abs() );
	}
    avr /= ( n * n );
    ASSERT_NEAR( avr, pow( 10, w / 10 ), pow( 10, w / 10 ) * 1e-2 );

}
