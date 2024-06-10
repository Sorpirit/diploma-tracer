#include "StatisicUILayer.hpp"

#include <imgui.h>
#include <string>

#include <glm/gtc/type_ptr.hpp>

namespace TracerCore::UI
{
    StatisticsWindow::StatisticsWindow(FrameStatisics &stats) : _stats(stats)
    {
    }

    StatisticsWindow::~StatisticsWindow()
    {
    }
    
    void StatisticsWindow::Render()
    {
        ImGui::Begin("Controls", &isOpen, ImGuiBackendFlags_None);
        std::string frameTime = "Frame time: " + std::to_string(_stats.FrameTime * 1000);
        std::string fpsTime = "FPS: " + std::to_string(_stats.FPS);
        std::string triCount = "Tri count: " + std::to_string(_stats.TriCount);
        ImGui::Text(fpsTime.c_str());
        ImGui::Text(frameTime.c_str());
        ImGui::Text(triCount.c_str());
        ImGui::ColorEdit4("Sky Color", glm::value_ptr(_stats.color));
        ImGui::SliderInt("Bounce count", &_stats.bounceCount, 2, 16);
        ImGui::End();
    }
}