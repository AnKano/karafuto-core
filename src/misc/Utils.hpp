#pragma once

#include <vector>
#include <cstdint>
#include <map>
#include <any>

static std::vector<uint8_t> anythingToByteVector(void *data, uint64_t length, uint64_t elementSize) {
    std::vector<uint8_t> convertedVtxPosData;
    auto *begin_ptr = reinterpret_cast<uint8_t *>(data);
    convertedVtxPosData.assign(begin_ptr, begin_ptr + (length * elementSize));
    return convertedVtxPosData;
}

template<typename F, typename S>
static std::vector<F> mapKeysDifference(const std::map<F, S> &fMap, const std::map<F, S> &sMap) {
    auto result = std::vector<F>{};

    auto collector = std::map<F, uint8_t>{};
    for (const auto &[key, _]: fMap) collector[key] = 0;
    for (const auto &[key, _]: sMap) collector[key] = 0;

    for (const auto &[key, _]: fMap) collector[key]++;
    for (const auto &[key, _]: sMap) collector[key]++;

    for (const auto &[key, _]: collector)
        if (collector[key] == 1) result.push_back(key);

    return result;
}

template<typename F, typename S>
static std::vector<F> mapKeysIntersection(const std::map<F, S> &fMap, const std::map<F, S> &sMap) {
    auto result = std::vector<F>{};

    auto collector = std::map<F, uint8_t>{};
    for (const auto &[key, _]: fMap) collector[key] = 0;
    for (const auto &[key, _]: sMap) collector[key] = 0;

    for (const auto &[key, _]: fMap) collector[key]++;
    for (const auto &[key, _]: sMap) collector[key]++;

    for (const auto &[key, _]: collector)
        if (collector[key] == 2) result.push_back(key);

    return result;
}