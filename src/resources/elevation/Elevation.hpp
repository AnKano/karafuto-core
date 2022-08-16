#pragma once

#include <vector>

namespace KCore {
    class Elevation {
    private:
        std::vector<std::vector<float>> mKernel;

    public:
        explicit Elevation(std::vector<std::vector<float>> kernel);

        [[nodiscard]]
        const float &get(const int &i, const int &j) const;

        [[nodiscard]]
        std::vector<float> getRow(const int &i) const;

        [[nodiscard]]
        std::vector<float> getColumn(const int &j) const;
    };
}

