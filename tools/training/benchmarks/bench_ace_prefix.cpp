#include <chrono>
#include <iostream>
#include <vector>

#include "tools/training/ace/ace_compressor.h"
#include "tools/training/ace/ace_compressors.h"
#include "tools/training/ace/ace_prefix_evaluator.h"
#include "tools/training/utils/thread_pool.h"

using namespace openzl;
using namespace openzl::training;

int main()
{
    std::cout << "========================================================\n";
    std::cout << "BENCHMARKING ACE CANDIDATE EVALUATOR PERFORMANCE\n";
    std::cout << "========================================================\n\n";

    // Generate test input sample (synthetic numeric sequence)
    std::vector<int32_t> data(100000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int32_t>(i % 256) + (i / 1000);
    }

    std::vector<Input> inputs;
    inputs.push_back(Input::refNumeric(data.data(), data.size()));

    // Generate candidate population
    std::mt19937_64 rng(42);
    size_t popSize = 100;
    std::vector<ACECompressor> population;

    auto prebuilt = getPrebuiltCompressors(Type::Numeric);
    population.insert(population.end(), prebuilt.begin(), prebuilt.end());

    while (population.size() < popSize) {
        population.push_back(buildRandomCompressor(rng, Type::Numeric));
    }

    ThreadPool threadPool(8);

    // Baseline Direct Benchmarking
    std::cout << "Running Baseline Direct Benchmarking on " << population.size() << " candidates...\n";
    auto startDirect = std::chrono::high_resolution_clock::now();
    std::vector<poly::optional<ACECompressionResult>> directResults(population.size());
    
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < population.size(); ++i) {
        futures.emplace_back(threadPool.run([&population, &inputs, &directResults, i] {
            directResults[i] = population[i].benchmark(inputs);
        }));
    }
    for (auto& f : futures) f.get();
    auto stopDirect = std::chrono::high_resolution_clock::now();
    double directMs = std::chrono::duration<double, std::milli>(stopDirect - startDirect).count();

    std::cout << "  Direct Benchmarking Execution Time: " << directMs << " ms\n\n";

    // Prefix Trie Work-Sharing Evaluator
    std::cout << "Running ACEPrefixEvaluator (Prefix-Tree Work-Sharing) on " << population.size() << " candidates...\n";
    auto startTrie = std::chrono::high_resolution_clock::now();
    auto trieResults = ACEPrefixEvaluator::evaluatePopulation(population, inputs, threadPool);
    auto stopTrie = std::chrono::high_resolution_clock::now();
    double trieMs = std::chrono::duration<double, std::milli>(stopTrie - startTrie).count();

    std::cout << "  ACEPrefixEvaluator Execution Time: " << trieMs << " ms\n\n";

    // Verification & Exact Match Check
    size_t matchCount = 0;
    size_t totalValid = 0;

    for (size_t i = 0; i < population.size(); ++i) {
        if (directResults[i].has_value()) {
            totalValid++;
            if (trieResults[i].has_value()) {
                if (directResults[i]->compressedSize == trieResults[i]->compressedSize) {
                    matchCount++;
                }
            }
        }
    }

    std::cout << "========================================================\n";
    std::cout << "VERIFICATION & ACCURACY SUMMARY:\n";
    std::cout << "========================================================\n";
    std::cout << "  Total Valid Candidates Evaluated : " << totalValid << "\n";
    std::cout << "  Exact Compressed Size Matches    : " << matchCount << " / " << totalValid << " ("
              << (totalValid > 0 ? (100.0 * matchCount / totalValid) : 0.0) << "%)\n";
    std::cout << "  Baseline Wall-Clock Time          : " << directMs << " ms\n";
    std::cout << "  Prefix Evaluator Wall-Clock Time  : " << trieMs << " ms\n";
    if (directMs > 0) {
        std::cout << "  Speedup Factor                    : " << (directMs / trieMs) << "x\n";
    }
    std::cout << "========================================================\n";

    return 0;
}
