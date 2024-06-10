#include "FrameControllsUI.hpp"
#include <imgui.h>

namespace TracerCore::UI
{
    FrameControllsUI::FrameControllsUI(VulkanDevice& device, const Window& window, SceneData& sceneData, Resources::Texture2D* screenTexture) :
        _device(device),
        _fileDialog(window.GetGLFWWindow()), 
        _sceneData(sceneData),
        _screenTexture(screenTexture)
    {
    }

    FrameControllsUI::~FrameControllsUI()
    {
    }

    void FrameControllsUI::Render()
    {
        ImGui::Begin("Frame Controlls");

        if(ImGui::Button("Load Model..."))
        {
            auto model = _fileDialog.OpenFile("3D Model (*.obj, *.fbx)\0*.obj;*.fbx\0");
            if(!model.empty())
            {
                _sceneData.ModelPath = model;
                _sceneData.IsSceneLoaded = false;
            }
        }

        int accSctructureType = _accSctructureConfig;
        ImGui::Combo("Acceleration Structure", &accSctructureType, "BVH SAH\0BVH Primitive\0Kd tree SHA\0Kd tree Primitive\0None\0");
        if(accSctructureType != _accSctructureConfig)
        {
            _accSctructureConfig = accSctructureType;
            switch (_accSctructureConfig)
            {
            case 0:
                _sceneData.AccStructureType = AccStructureType::AccStructure_BVH;
                _sceneData.AccHeruishitcType = AccHeruishitcType::AccHeruishitc_SAH;
                break;
            case 1:
                _sceneData.AccStructureType = AccStructureType::AccStructure_BVH;
                _sceneData.AccHeruishitcType = AccHeruishitcType::AccHeruishitc_Primitive;
                break;
            case 2:
                _sceneData.AccStructureType = AccStructureType::AccStructure_KdTree;
                _sceneData.AccHeruishitcType = AccHeruishitcType::AccHeruishitc_SAH;
                break;
            case 3:
                _sceneData.AccStructureType = AccStructureType::AccStructure_KdTree;
                _sceneData.AccHeruishitcType = AccHeruishitcType::AccHeruishitc_Primitive;
                break;
            case 4:
                _sceneData.AccStructureType = AccStructureType::AccStructure_None;
                break;

            default:
                break;
            }
            
            _sceneData.IsSceneLoaded = false;
        }

        if(ImGui::Button("Save Screen Shot..."))
        {
            auto piccturePath = _fileDialog.SaveFile("PNG (*.png)\0*.png\0");
            if(!piccturePath.empty())
            {
                Resources::Texture2D::SaveTextureToFile(piccturePath, _screenTexture, _device);
            }
        }

        if(ImGui::Button("Reset Camera"))
        {
            _sceneData.ResetCamera = true;
        }

        ImGui::End();
    }

    void FrameControllsUI::LoadModelPaths()
    {

    }

} // namespace TracerCore::UI
