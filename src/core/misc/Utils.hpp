#pragma once

#include <vector>
#include <cstdint>

static std::vector<uint8_t> AnythingToByteVector(void* data, uint64_t length, uint64_t elementSize) {
    std::vector<uint8_t> convertedVtxPosData;
    auto* begin_ptr = reinterpret_cast<uint8_t*>(data);
    convertedVtxPosData.assign(begin_ptr, begin_ptr + (length * elementSize));
    return convertedVtxPosData;
}