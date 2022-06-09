#pragma once

#include "../../ISource.hpp"

namespace KCore {

    class SRTMSource : public ISource {
    public:
        double mXOrigin{}, mYOrigin{};
        double mXOpposite{}, mYOpposite{};
        double mPixelWidth{}, mPixelHeight{};

    public:
        SRTMSource(const char *path, SourceType type) : ISource(path, type) {
            restoreMetaFromFilename();
        }

    private:
        static int8_t parseSRTMSignByIterator(const std::string &string, uint64_t &position, uint8_t stage) {
            if (position > string.length()) throw std::runtime_error("Found EOL in filename!");

            auto ch = string[position++];

            switch (stage) {
                case 1:
                    if (ch == 'N') return 1;
                    if (ch == 'S') return -1;
                case 2:
                    if (ch == 'E') return 1;
                    if (ch == 'W') return -1;
                default: throw std::runtime_error("Wrong char in filename!");
            }
        }

        static uint16_t parseNumberByIterator(const std::string &string, uint64_t &position) {
            if (position > string.length()) throw std::runtime_error("Found EOL in filename!");

            std::stringstream collector;

            while (position <= string.length() && string[position] >= '0' && string[position] <= '9') {
                collector << string[position];
                position++;
            }

            collector.seekg(0, std::ios::end);
            if (!collector.tellg()) throw std::runtime_error("Wrong number in filename!");

            collector.seekg(0, std::ios::beg);

            uint16_t value = 0.0;
            collector >> value;
            return value;
        }

        void restoreMetaFromFilename() {
            const int stages = 2;

            uint64_t pos = 0;

            std::array<int, stages> values{};
            for (int i = 0; i < stages; i++) {
                parseSRTMSignByIterator(mFileNameBase, pos, i + 1);
                values[i] = parseNumberByIterator(mFileNameBase, pos);
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