#include <gtest/gtest.h>
#include "dsp/fft.h"
#include "receiver/gen_noise.h"
#include "base/writer.h"
#include <vector>
#include <fstream>

using namespace Base;

TEST( TestNoise, WNFine ) {
	Fftw fftw;
	GenNoise GenN;
	float W = 10;
	uint64_t N = 4096 * 16;

	std::vector< Complex< float > > data_ = GenN.GenWN< float >( W, N );

    fftw.Forward< float >( data_.data(), data_.data(), data_.size() );

	float avr = 0;
	for( uint64_t i = 0; i < N; i++ ) {
		avr += ( data_[ i ].abs() * data_[ i ].abs() );
	}
    avr /= (N * N);
    ASSERT_NEAR( avr, pow( 10, W / 10 ), pow( 10, W / 10 ) * 1e-2 );

}
