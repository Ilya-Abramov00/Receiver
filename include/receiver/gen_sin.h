#ifndef GEN_SIN_H
#define GEN_SIN_H

#include <vector>
#include "base/complex.h"
#include <cmath>

using namespace Base;

class Generator_sin { // определение сюда тк у шаблонных классов обявление и реализация должна быть в 1 файле

public:

    template < typename Type >
    std::vector< Complex< Type > > gen_sin( float amp, uint64_t freq, uint32_t fd,  uint32_t s_len ) {

        if( s_len == 0 ) {
            throw std::runtime_error( "length cant be equal 0" );
        }
        if( fd == 0 ) {

            throw std::runtime_error( "Fd cant be equal 0" );
        }

        std::vector< Complex< Type > > data( s_len );
        for( uint32_t i = 0; i < s_len; i++ ) {

            data [ i ] = { static_cast< Type >( amp * cos( 2 * M_PI * freq * i / fd ) ),
                       static_cast< Type >( amp * sin( 2 * M_PI * freq * i / fd ) ) };

        }
        return data;

    }

private:

};

#endif // GEN_SIN_H
