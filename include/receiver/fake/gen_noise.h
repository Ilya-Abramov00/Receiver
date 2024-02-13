#ifndef GEN_NOISE_H
#define GEN_NOISE_H

#include "base/complex.h"

#include <cstdint>
#include <ctime>
#include <vector>

using namespace Base;

class GenNoise {
public:
    template<typename Type>
    std::vector<Complex<Type> > GenWN(float W, uint32_t size) {
        srand(time(0));
        double A = sqrt(1.5) * pow(10, W / 20); // try double
        std::vector<Complex<Type> > data(size);

        for(uint32_t i = 0; i < size; i++) {
            data[i] = {static_cast<Type>(A * (2 * ((rand() / (static_cast<double>(RAND_MAX))) - 0.5))),
                       static_cast<Type>(A * (2 * ((rand() / (static_cast<double>(RAND_MAX))) - 0.5)))};
        }
        return data;
    }

private:
};

#endif // GEN_NOISE_H
