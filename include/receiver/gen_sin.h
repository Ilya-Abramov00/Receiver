#ifndef GEN_SIN_H
#define GEN_SIN_H

#include "base/complex.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

using namespace Base;

class GeneratorSin { // определение сюда тк у шаблонных классов обявление и реализация должна быть в 1 файле

public:
    std::vector<Complex<float> > genSin(float amp, uint64_t freq, uint32_t fd, uint32_t s_len) {
        if(s_len == 0) {
            throw std::runtime_error("length cant be equal 0");
        }
        if(fd == 0) {
            throw std::runtime_error("Fd cant be equal 0");
        }

        std::vector<Complex<float> > data(s_len);
        for(uint32_t i = 0; i < s_len; i++) {
            data[i] = {static_cast<float>(amp * cos(2 * M_PI * freq * i / fd)),
                       static_cast<float>(amp * sin(2 * M_PI * freq * i / fd))};
        }

        return data;
    }

private:
};

#endif // GEN_SIN_H
