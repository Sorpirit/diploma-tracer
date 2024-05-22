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

        glm::vec3 color; 
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