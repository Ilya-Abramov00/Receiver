#include "base/complex.h"
#include "receiver/fake/gen_sin.h"

#include <fstream>
#include <gtest/gtest.h>
#include <vector>

TEST(genSinTest, output_values) {
    std::ifstream file("bin/test_val_sin", std::ifstream::binary);
    if(!file.is_open()) {
        throw std::runtime_error("cant open file ");
    }

    std::vector<Complex<float> > vec(1000);

    for(uint32_t i = 0; i < 1000; i++) {
        file.read(reinterpret_cast<char*>(&(vec[i].re)), sizeof(float));
        file.read(reinterpret_cast<char*>(&(vec[i].im)), sizeof(float));
    }
    file.close();

    GeneratorSin test_obj;

    std::vector<Complex<float> > data = test_obj.genSin(1, 10, 1600, 1000);

    for(uint64_t i = 0; i < 1000; ++i) {
        ASSERT_NEAR(data[i].re, vec[i].re, 1e-10);
        ASSERT_NEAR(data[i].im, vec[i].im, 1e-10);
    }
}

TEST(sin_test, null_size) {
    GeneratorSin test_obj;

    try {
        std::vector<Complex<float> > data = test_obj.genSin(1, uint64_t(10), 1600, 0);

    } catch(std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "length cant be equal 0") << "something wrong\n";
    }
}

TEST(sin_test, null_fd) {
    GeneratorSin test_obj;

    try {
        std::vector<Complex<float> > data = test_obj.genSin(1, 10, 0, 100);

    } catch(std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Fd cant be equal 0") << "something wrong\n";
    }
}
