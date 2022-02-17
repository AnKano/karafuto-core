#pragma once

#include "Stage.hpp"

namespace KCore::BuiltInStages {
    KCore::Stage *CommonCalculate();

    KCore::Stage *ImageCalculate();

    KCore::Stage *JSONCalculate();
}