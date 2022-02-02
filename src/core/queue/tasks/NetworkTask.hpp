#pragma once

#include <string>
#include <utility>
#include <iostream>

#include <curl/curl.h>

#include "BaseTask.hpp"

namespace KCore {
    struct NetworkTask {
    public:
        std::string mUrl;
        std::string mQuadcode;
        std::string mTag;

        NetworkTask(std::string url, std::string quadcode, std::string tag) :
                mUrl(std::move(url)), mQuadcode(std::move(quadcode)), mTag(std::move(tag)) {}
    };
}