#include "Elevation.hpp"

namespace KCore {
    Elevation::Elevation
            (std::vector<std::vector<float>> kernel) : mKernel(std::move(kernel)) {}

    const float &Elevation::get
            (const int &i, const int &j) const {
        return mKernel[i][j];
    }

    std::vector<float> Elevation::getRow
            (const int &i) const {
        return mKernel[i];
    }

    std::vector<float> Elevation::getColumn
            (const int &j) const {
        auto collector = std::vector<float>{};
        for (const auto &item: mKernel)
            collector.push_back(item[j]);

        return collector;
    }
}