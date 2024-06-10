#pragma once

#include <string>
#include <unordered_map>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include "../Window.hpp"
#include "UILayer.hpp"
#include "../TracerScene.hpp"
#include "WindowsFileDialog.hpp"
#include "../Resources/Texture2D.hpp"

namespace TracerCore::UI
{

    struct SceneData
    {
        bool IsSceneLoaded;
        bool ResetCamera;
        std::string ModelPath;
        AccStructureType AccStructureType;
        AccHeruishitcType AccHeruishitcType;
    };

    class FrameControllsUI : public RenderUILayer
    {
    public:
        FrameControllsUI(VulkanDevice& device, const Window& window, SceneData& sceneData, Resources::Texture2D* screenTexture);
        ~FrameControllsUI();

        void Render() override;

    private:
        void LoadModelPaths();

        VulkanDevice& _device;

        SceneData& _sceneData;
        int _accSctructureConfig = 0;

        WindowsFileDialog _fileDialog;
        std::unordered_map<std::string, std::string> _model_paths;
        Resources::Texture2D* _screenTexture;
    };
} // namespace TracerCore::UI