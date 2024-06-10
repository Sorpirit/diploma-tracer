#pragma once

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

#include "UILayer.hpp"

#include "glm/glm.hpp"

namespace TracerCore::UI
{
    struct FrameStatisics 
    {
        float FrameTime;
        uint32_t FPS;
        uint32_t TriCount;

        //todo remove
        glm::vec3 color; 
        int bounceCount;
    };

    class StatisticsWindow : public RenderUILayer
    {
        public:
            StatisticsWindow(FrameStatisics& stats);
            ~StatisticsWindow();

            void Render() override;
        private:
            FrameStatisics& _stats;
            bool isOpen = true;
    };
}