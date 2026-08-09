// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "tools/training/ace/automated_compressor_explorer.h"

#include <chrono>
#include <vector>

#include "openzl/common/a1cbor_helpers.h"
#include "openzl/common/allocation.h"
#include "openzl/shared/a1cbor.h"
#include "tools/training/ace/ace_prefix_evaluator.h"

namespace openzl {
namespace training {

std::vector<ACECompressor> AutomatedCompressorExplorer::initialPopulation()
{
    // Use all prebuilt compressors
    auto prebuilt = getPrebuiltCompressors(inputType());
    std::vector<ACECompressor> population;
    population.insert(population.end(), prebuilt.begin(), prebuilt.end());

    // Use populationSize() random compressors
    for (size_t i = 0; i < populationSize(); ++i) {
        population.push_back(buildRandomCompressor(rng(), inputType()));
    }
    return population;
}

std::unordered_set<ACECompressor> AutomatedCompressorExplorer::reproduce(
        size_t numChildren)
{
    std::unordered_set<ACECompressor> children;
    size_t maxIterations = 2 * numChildren;
    std::vector<std::future<ACECompressor>> futures;
    futures.reserve(maxIterations);

    for (size_t i = 0; i < maxIterations; ++i) {
        uint64_t seed = rng()();
        futures.emplace_back(threadPool_.run([this, seed]() -> ACECompressor {
            std::mt19937_64 childRng(seed);
            TournamentSelector childSelector(
                    TournamentSelector::Parameters{ .seed = childRng() });

            auto selectParentLocal = [&]() -> const ACECompressor& {
                auto idx = childSelector.select(rank(), crowdingDistance());
                return population()[idx];
            };

            const auto& p1 = selectParentLocal();
            const auto& p2 = selectParentLocal();

            ACECrossover crossoverLocal(childRng, inputType());
            ACEMutate mutateLocal(childRng, inputType());

            auto child = crossoverLocal(p1, p2);
            std::uniform_real_distribution<float> shouldMutate(0, 1);
            if (shouldMutate(childRng) < mutationProbability()) {
                child = mutateLocal(child);
            }
            return child;
        }));
    }

    for (size_t i = 0; i < maxIterations; ++i) {
        auto child = futures[i].get();
        if (children.size() < numChildren
            && populationSet().count(child) == 0) {
            children.insert(std::move(child));
        }
    }
    return children;
}

namespace {
/// Put small evolutionary pressure on the results to be simpler.
/// Adjusts cSize on the scale of 0.1%, and times on the scale of 1%, given a
/// typical number of components of ~10.
void adjustResults(const ACECompressor& gene, std::vector<float>& results)
{
    std::array<float, 3> scale = { 0.0001, 0.001, 0.001 };
    const size_t numComponents = gene.numComponents();
    for (size_t i = 0; i < results.size(); ++i) {
        const auto delta = results[i] * scale[i] * numComponents;
        results[i] += std::max<float>(numComponents, delta);
    }
}
} // namespace

/* static */ std::vector<float> AutomatedCompressorExplorer::computeFitness(
        const ACECompressor& gene,
        poly::span<const Input> inputs)
{
    auto result = gene.benchmark(inputs);
    std::vector<float> fitness(3, std::numeric_limits<float>::infinity());
    if (result.has_value()) {
        fitness[0] = result->compressedSize;
        fitness[1] = result->compressionTime.count();
        fitness[2] = result->decompressionTime.count();
        adjustResults(gene, fitness);
    }
    return fitness;
}

std::vector<float> AutomatedCompressorExplorer::computeFitness(
        const ACECompressor& gene)
{
    return computeFitness(gene, inputs_);
}

double AutomatedCompressorExplorer::progress() const
{
    double subGen = 0.0;
    size_t const total = totalCandidates_.load();
    if (total > 0) {
        subGen = static_cast<double>(completedCandidates_.load()) / static_cast<double>(total);
    }
    double const generationProgress =
            (static_cast<double>(generation()) + subGen) / static_cast<double>(maxGenerations());

    if (!deadline().has_value()) {
        return std::min(1.0, generationProgress);
    }

    std::chrono::nanoseconds const remaining =
            deadline().value() - std::chrono::steady_clock::now();
    std::chrono::nanoseconds const maxTime = parameters().maxTime.value();
    double const timeProgress = 1.0 - (double)remaining.count() / maxTime.count();

    return std::min(1.0, std::max(generationProgress, timeProgress));
}

std::vector<std::vector<float>> AutomatedCompressorExplorer::computeFitness(
        poly::span<const ACECompressor> genes)
{
    totalCandidates_     = genes.size();
    completedCandidates_ = 0;

    // Check fitness cache first for known candidate hashes
    std::vector<poly::optional<std::vector<float>>> cachedResults(genes.size(), poly::nullopt);
    std::vector<size_t> uncachedIndices;

    for (size_t i = 0; i < genes.size(); ++i) {
        auto cache = cachedFitness_.find(genes[i].hash());
        if (cache != cachedFitness_.end()) {
            std::uniform_int_distribution<size_t> dist(0, cache->second.first);
            if (dist(rng()) != 0) {
                cachedResults[i] = cache->second.second;
                continue;
            }
        }
        uncachedIndices.push_back(i);
    }

    // Evaluate uncached candidates via prefix-tree work sharing with inter-generation history
    std::vector<ACECompressor> uncachedGenes;
    uncachedGenes.reserve(uncachedIndices.size());
    for (auto idx : uncachedIndices) {
        uncachedGenes.push_back(genes[idx]);
    }

    auto evalResults = ACEPrefixEvaluator::evaluatePopulation(
            uncachedGenes, inputs_, threadPool_, evaluatedHistory_);

    // Update history for subsequent generations (keep up to 500 candidates)
    evaluatedHistory_.insert(
            evaluatedHistory_.end(), uncachedGenes.begin(), uncachedGenes.end());
    if (evaluatedHistory_.size() > 500) {
        evaluatedHistory_.erase(
                evaluatedHistory_.begin(),
                evaluatedHistory_.begin() + (evaluatedHistory_.size() - 500));
    }

    std::vector<std::vector<float>> results(genes.size());
    size_t uncachedIdx = 0;

    for (size_t i = 0; i < genes.size(); ++i) {
        if (cachedResults[i].has_value()) {
            results[i] = *cachedResults[i];
        } else {
            const auto& res = evalResults[uncachedIdx++];
            std::vector<float> fitness(3, std::numeric_limits<float>::infinity());
            if (res.has_value()) {
                fitness[0] = res->compressedSize;
                fitness[1] = res->compressionTime.count();
                fitness[2] = res->decompressionTime.count();
                adjustResults(genes[i], fitness);
            }
            results[i] = fitness;

            auto [cache, inserted] = cachedFitness_.emplace(
                    genes[i].hash(), std::make_pair(1, fitness));
            if (!inserted) {
                for (size_t j = 0; j < fitness.size(); ++j) {
                    cache->second.second[j] =
                            std::min(cache->second.second[j], fitness[j]);
                }
                ++cache->second.first;
            }
        }
        completedCandidates_++;
        if (progressCallback_) {
            progressCallback_(static_cast<float>(progress()));
        }
    }
    return results;
}

namespace {
struct ArenaDeleter {
    void operator()(Arena* arena) const
    {
        ALLOC_Arena_freeArena(arena);
    }
};
} // namespace

std::string AutomatedCompressorExplorer::savePopulation() const
{
    std::unique_ptr<Arena, ArenaDeleter> arena(ALLOC_HeapArena_create());
    auto a         = A1C_Arena_wrap(arena.get());
    A1C_Item* item = A1C_Item_root(&a);
    if (item == nullptr) {
        throw std::bad_alloc();
    }
    A1C_Item* array = A1C_Item_array(item, population().size(), &a);
    if (array == nullptr) {
        throw std::bad_alloc();
    }
    for (const auto& gene : population()) {
        const auto s = gene.serialize();
        if (!A1C_Item_string_copy(array++, s.data(), s.size(), &a)) {
            throw std::bad_alloc();
        }
    }
    const size_t size = A1C_Item_encodedSize(item);
    std::string serialized(size, '\0');
    const size_t actual = A1C_Item_encode(
            item, (uint8_t*)serialized.data(), serialized.size(), nullptr);
    if (size != actual) {
        throw Exception("Serialization failed");
    }
    return serialized;
}

void AutomatedCompressorExplorer::loadPopulation(poly::string_view snapshot)
{
    std::unique_ptr<Arena, ArenaDeleter> arena(ALLOC_HeapArena_create());
    A1C_Decoder decoder;
    A1C_Decoder_init(&decoder, A1C_Arena_wrap(arena.get()), {});
    const A1C_Item* item = A1C_Decoder_decode(
            &decoder, (const uint8_t*)snapshot.data(), snapshot.size());
    if (item == nullptr) {
        throw Exception("Failed to deserialize");
    }
    if (item->type != A1C_ItemType_array) {
        throw Exception("Must be an array");
    }
    std::vector<ACECompressor> population;
    population.reserve(item->array.size);
    for (size_t i = 0; i < item->array.size; ++i) {
        const A1C_Item& p = item->array.items[i];
        if (p.type != A1C_ItemType_string) {
            throw Exception("Array elements must be strings");
        }
        population.emplace_back(
                poly::string_view{ p.string.data, p.string.size });
    }
    extendPopulation(std::move(population));
}

} // namespace training
} // namespace openzl
