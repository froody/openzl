// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

#include "openzl/codecs/octree_occupancy/decode_octree_occupancy_kernel.h"
#include "openzl/codecs/octree_occupancy/encode_octree_occupancy_kernel.h"

namespace {

void testRoundTrip(const uint8_t* input, size_t size) {
    size_t bound = ZS_octreeOccupancy_encodeBound(size);
    std::vector<uint8_t> encoded(bound);

    size_t written = ZS_octreeOccupancy_encode(encoded.data(), bound, input, size);
    if (written == 0) {
        assert(false && "Encode failed");
    }

    size_t decodedSize = ZS_octreeOccupancy_getDecodedSize(encoded.data(), written);
    assert(decodedSize == size);

    std::vector<uint8_t> decoded(decodedSize + 16, 0xAA);
    size_t actualDecoded = ZS_octreeOccupancy_decode(decoded.data(), decodedSize, encoded.data(), written);
    assert(actualDecoded == size);
    (void)actualDecoded;

    if (size > 0) {
        assert(std::memcmp(input, decoded.data(), size) == 0);
    }
}

void runSyntheticTests() {
    // 1. Empty input
    testRoundTrip(nullptr, 0);

    // 2. Small inputs
    uint8_t smallInput[] = {1, 2, 3, 4, 5};
    testRoundTrip(smallInput, sizeof(smallInput));

    // 3. 3D uint8_t points (3 bytes each)
    std::vector<uint8_t> points8 = {
        10, 20, 30,
        15, 25, 35,
        100, 200, 150,
        10, 20, 30 // duplicate
    };
    testRoundTrip(points8.data(), points8.size());

    // 4. 3D uint16_t points (6 bytes each)
    std::vector<uint16_t> points16 = {
        1000, 2000, 3000,
        5000, 1000, 2000,
        1000, 2000, 3000 // duplicate
    };
    testRoundTrip(reinterpret_cast<const uint8_t*>(points16.data()), points16.size() * sizeof(uint16_t));

    // 5. 3D uint32_t points (12 bytes each)
    std::vector<uint32_t> points32 = {
        100000, 200000, 300000,
        500000, 100000, 200000,
        900000, 800000, 700000,
        100000, 200000, 300000 // duplicate
    };
    testRoundTrip(reinterpret_cast<const uint8_t*>(points32.data()), points32.size() * sizeof(uint32_t));

    // 6. Identical points
    std::vector<uint32_t> identical32(30, 42);
    testRoundTrip(reinterpret_cast<const uint8_t*>(identical32.data()), identical32.size() * sizeof(uint32_t));
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    testRoundTrip(data, size);
    return 0;
}

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
int main(int argc, char** argv) {
    std::cout << "Running Octree Occupancy Synthetic Tests..." << std::endl;
    runSyntheticTests();

    std::cout << "Running Random Fuzz Inputs..." << std::endl;
    uint32_t seed = 12345;
    for (int iter = 0; iter < 1000; ++iter) {
        size_t len = (seed % 1024);
        std::vector<uint8_t> randData(len);
        for (size_t i = 0; i < len; ++i) {
            seed = seed * 1103515245 + 12345;
            randData[i] = static_cast<uint8_t>(seed >> 16);
        }
        testRoundTrip(randData.data(), len);
    }

    std::cout << "All 100% Round-Trip Correctness Tests PASSED Successfully!" << std::endl;
    return 0;
}
#endif
