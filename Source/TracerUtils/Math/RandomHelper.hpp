#pragma once

#include <cstdlib> 

namespace TracerUtils::Math
{
    class TracerRandom
    {
        public:
            static float RandomFloat(float min, float max) {
                if (!_seeded) {
                    srand(static_cast<unsigned int>(time(0)));
                    _seeded = true;
                }

                return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
            }

        private:
            TracerRandom() = delete;

            static bool _seeded;
    };

    bool TracerRandom::_seeded = false;
}