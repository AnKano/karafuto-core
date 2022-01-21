#pragma once

#include <vector>
#include <cstdint>

static std::vector<uint8_t> ConvertToBytes(void* data, uint64_t length, uint64_t elementSize) {
    std::vector<uint8_t> convertedVtxPosDdata;
    auto* ibegin = reinterpret_cast<uint8_t*>(data);
    convertedVtxPosDdata.assign(ibegin, ibegin + (length * elementSize));
    return convertedVtxPosDdata;
}