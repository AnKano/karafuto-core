#pragma once

#include "BaseFileSource.hpp"

#include <sstream>
#include <iostream>

namespace KCore {
    class SRTMFileSource : public BaseFileSource {
        double mXOrigin{}, mYOrigin{};
        double mXOpposite{}, mYOpposite{};
        double mPixelWidth{}, mPixelHeight{};

    public:
        SRTMFileSource(const std::string &path) : BaseFileSource(path) {
            try {
                parseMeta(mFileNameBase);
            } catch (const std::exception &e) {
                mFileCorrupted = true;
                std::cerr << e.what() << std::endl;
            }
        }

        std::vector<uint8_t> getRawData() override {
            return mData;
        }

        uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) override {
            return nullptr;
        }

    private:
        static std::optional<int8_t> parseSRTMSignByIterator(
                const std::string &string,
                std::_String_iterator<std::_String_val<std::_Simple_types<char>>> &it,
                uint8_t stage
        ) {
            if (it == string.end())
                throw std::runtime_error("Found EOL in filename!");

            auto ch = *(it++);

            switch (stage) {
                case 1:
                    if (ch == 'N')
                        return 1;
                    if (ch == 'S')
                        return -1;
                case 2:
                    if (ch == 'E')
                        return 1;
                    if (ch == 'W')
                        return -1;
                default:
                    throw std::runtime_error("Wrong char in filename!");
            }
        }

        static uint16_t parseNumberByIterator(
                const std::string &string,
                std::_String_iterator<std::_String_val<std::_Simple_types<char>>> &it
        ) {
            if (it == string.end()) throw std::runtime_error("Found EOL in filename!");

            std::stringstream collector;

            while (it != string.end() && *it >= '0' && *it <= '9') {
                collector << *it;
                it++;
            }

            collector.seekg(0, std::ios::end);
            if (!collector.tellg())
                throw std::runtime_error("Wrong number in filename!");

            collector.seekg(0, std::ios::beg);

            uint16_t value = 0.0;
            collector >> value;
            return value;
        }

        void parseMeta(std::string &fileNameBase) {
            auto it = fileNameBase.begin();

            const int stages = 2;
            std::array<int, stages> values{};
            for (int i = 0; i < stages; i++) {
                auto sign = parseSRTMSignByIterator(fileNameBase, it, i + 1);
                if (!sign.has_value()) throw std::runtime_error("Filename corrupted!");

                values[i] = parseNumberByIterator(fileNameBase, it);
            }

            mYOrigin = values[0] + 1.0;
            mYOpposite = values[0];

            mXOrigin = values[1];
            mXOpposite = values[1] + 1.0;

            mPixelWidth = 1.0 / 3601.0;
            mPixelHeight = mPixelWidth;
        };
    };
}