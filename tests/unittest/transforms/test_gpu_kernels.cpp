// Copyright (c) Meta Platforms, Inc. and affiliates.

// The codec bindings pick between the CPU and the GPU kernel at runtime based
// on payload size, GPU availability, and whether a previous dispatch failed.
// That choice is only safe while both paths emit exactly the same bytes, so
// that is what these tests pin down.

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA

#    include <gtest/gtest.h>

#    include <cuda_runtime.h>
#    include <cstdint>
#    include <string>
#    include <vector>

#    include "openzl/codecs/delta/encode_delta_kernel.h"
#    include "openzl/codecs/gpu/encode_delta_gpu.h"
#    include "openzl/codecs/gpu/encode_zigzag_gpu.h"
#    include "openzl/codecs/zigzag/encode_zigzag_kernel.h"

namespace {

/// Device allocation that frees itself, so each case below stays readable.
template <typename T>
class DeviceBuffer {
   public:
    explicit DeviceBuffer(size_t nbElts)
    {
        if (nbElts > 0) {
            EXPECT_EQ(cudaSuccess, cudaMalloc(&ptr_, nbElts * sizeof(T)));
        }
    }
    ~DeviceBuffer()
    {
        cudaFree(ptr_);
    }
    DeviceBuffer(DeviceBuffer const&)            = delete;
    DeviceBuffer& operator=(DeviceBuffer const&) = delete;

    T* get() const
    {
        return ptr_;
    }

    void copyFrom(std::vector<T> const& host)
    {
        ASSERT_EQ(
                cudaSuccess,
                cudaMemcpy(
                        ptr_,
                        host.data(),
                        host.size() * sizeof(T),
                        cudaMemcpyHostToDevice));
    }

    std::vector<T> toHost(size_t nbElts) const
    {
        std::vector<T> host(nbElts);
        EXPECT_EQ(
                cudaSuccess,
                cudaMemcpy(
                        host.data(),
                        ptr_,
                        nbElts * sizeof(T),
                        cudaMemcpyDeviceToHost));
        return host;
    }

   private:
    T* ptr_ = nullptr;
};

/// Values spanning the whole width, including the sign-bit patterns that
/// zigzag and delta are most likely to get wrong.
template <typename T>
std::vector<T> testData(size_t nbElts)
{
    std::vector<T> data(nbElts);
    uint64_t state = 0x243f6a8885a308d3ULL;
    for (size_t i = 0; i < nbElts; ++i) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        // Salt in the extremes so min/max/-1/0 are always covered.
        switch (i % 8) {
            case 0:
                data[i] = (T)0;
                break;
            case 1:
                data[i] = (T)~(uint64_t)0;
                break;
            case 2:
                data[i] = (T)((uint64_t)1 << (sizeof(T) * 8 - 1));
                break;
            case 3:
                data[i] = (T)(((uint64_t)1 << (sizeof(T) * 8 - 1)) - 1);
                break;
            default:
                data[i] = (T)state;
                break;
        }
    }
    return data;
}

/// True once the machine actually has a usable CUDA device. Without this the
/// tests would fail on CPU-only build machines that still compile the kernels.
bool hasCudaDevice()
{
    int count       = 0;
    cudaError_t err = cudaGetDeviceCount(&count);
    if (err != cudaSuccess) {
        // A failed probe leaves an error pending; clear it for later calls.
        cudaGetLastError();
        return false;
    }
    return count > 0;
}

// ---------------------------------------------------------------------------
// Zigzag
// ---------------------------------------------------------------------------

template <typename Signed, typename Unsigned>
void expectZigzagMatchesCpu(
        size_t nbElts,
        void (*cpuEncode)(Unsigned*, Signed const*, size_t),
        cudaError_t (*gpuEncode)(Unsigned*, Signed const*, size_t, cudaStream_t))
{
    SCOPED_TRACE(
            "zigzag width=" + std::to_string(sizeof(Signed))
            + " nbElts=" + std::to_string(nbElts));

    std::vector<Signed> const src = testData<Signed>(nbElts);

    std::vector<Unsigned> expected(nbElts);
    cpuEncode(expected.data(), src.data(), nbElts);

    DeviceBuffer<Signed> d_src(nbElts);
    DeviceBuffer<Unsigned> d_dst(nbElts);
    d_src.copyFrom(src);
    ASSERT_EQ(cudaSuccess, gpuEncode(d_dst.get(), d_src.get(), nbElts, nullptr));
    ASSERT_EQ(cudaSuccess, cudaStreamSynchronize(nullptr));

    EXPECT_EQ(expected, d_dst.toHost(nbElts));
}

// ---------------------------------------------------------------------------
// Delta
// ---------------------------------------------------------------------------

template <typename T>
void expectDeltaMatchesCpu(
        size_t nbElts,
        void (*cpuEncode)(T*, T*, T const*, size_t),
        cudaError_t (*gpuEncode)(T*, T*, T const*, size_t, cudaStream_t))
{
    SCOPED_TRACE(
            "delta width=" + std::to_string(sizeof(T))
            + " nbElts=" + std::to_string(nbElts));
    ASSERT_GT(nbElts, 0u);

    std::vector<T> const src = testData<T>(nbElts);

    T expectedFirst = 0;
    std::vector<T> expectedDeltas(nbElts - 1);
    cpuEncode(&expectedFirst, expectedDeltas.data(), src.data(), nbElts);

    DeviceBuffer<T> d_src(nbElts);
    DeviceBuffer<T> d_deltas(nbElts - 1);
    DeviceBuffer<T> d_first(1);
    d_src.copyFrom(src);
    ASSERT_EQ(
            cudaSuccess,
            gpuEncode(
                    d_first.get(), d_deltas.get(), d_src.get(), nbElts, nullptr));
    ASSERT_EQ(cudaSuccess, cudaStreamSynchronize(nullptr));

    EXPECT_EQ(expectedFirst, d_first.toHost(1)[0]);
    EXPECT_EQ(expectedDeltas, d_deltas.toHost(nbElts - 1));
}

// Sizes straddling the block size (256), the size at which the codec bindings
// start offloading (65536), and a non-multiple of both.
constexpr size_t kElementCounts[] = { 1, 2, 255, 256, 257, 65535, 65536, 100003 };

} // namespace

TEST(GpuKernelEquivalence, ZigzagEncodeMatchesCpu)
{
    if (!hasCudaDevice()) {
        GTEST_SKIP() << "no CUDA device";
    }
    for (size_t nbElts : kElementCounts) {
        expectZigzagMatchesCpu<int8_t, uint8_t>(
                nbElts, ZL_zigzagEncode8, ZL_zigzagEncode8_gpu);
        expectZigzagMatchesCpu<int16_t, uint16_t>(
                nbElts, ZL_zigzagEncode16, ZL_zigzagEncode16_gpu);
        expectZigzagMatchesCpu<int32_t, uint32_t>(
                nbElts, ZL_zigzagEncode32, ZL_zigzagEncode32_gpu);
        expectZigzagMatchesCpu<int64_t, uint64_t>(
                nbElts, ZL_zigzagEncode64, ZL_zigzagEncode64_gpu);
    }
}

TEST(GpuKernelEquivalence, DeltaEncodeMatchesCpu)
{
    if (!hasCudaDevice()) {
        GTEST_SKIP() << "no CUDA device";
    }
    for (size_t nbElts : kElementCounts) {
        expectDeltaMatchesCpu<uint8_t>(
                nbElts, ZS_deltaEncode8, ZS_deltaEncode8_gpu);
        expectDeltaMatchesCpu<uint16_t>(
                nbElts, ZS_deltaEncode16, ZS_deltaEncode16_gpu);
        expectDeltaMatchesCpu<uint32_t>(
                nbElts, ZS_deltaEncode32, ZS_deltaEncode32_gpu);
    }
}

// The kernels cap their grid and loop with a grid stride, so a payload with
// more elements than the capped grid has threads is the case where an index
// that is computed or advanced in 32-bit arithmetic goes wrong.
TEST(GpuKernelEquivalence, HandlesPayloadLargerThanTheGrid)
{
    if (!hasCudaDevice()) {
        GTEST_SKIP() << "no CUDA device";
    }
    // One element past 65535 blocks * 256 threads.
    constexpr size_t kBeyondGrid = 65535u * 256u + 1u;
    expectZigzagMatchesCpu<int8_t, uint8_t>(
            kBeyondGrid, ZL_zigzagEncode8, ZL_zigzagEncode8_gpu);
    expectDeltaMatchesCpu<uint8_t>(
            kBeyondGrid, ZS_deltaEncode8, ZS_deltaEncode8_gpu);
}

#endif // OPENZL_HAS_CUDA
