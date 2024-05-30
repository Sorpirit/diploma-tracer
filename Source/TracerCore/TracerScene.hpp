#pragma once

#include <vector>
#include <memory>

#include "VulkanDevice.hpp"
#include "Resources/VulkanBuffer.hpp"
#include "Models/TracerMesh.hpp"
#include "BHVTree.hpp"

namespace TracerCore
{
    class TracerScene
    {
    public:
        TracerScene(VulkanDevice& device);
        ~TracerScene();

        TracerScene(const TracerScene&) = delete;
        TracerScene &operator=(const TracerScene&) = delete;

        void AddModel(TracerUtils::Models::TracerMesh& model);
        void BuildScene();

        inline const BHVTree& GetBHVTree() const { return *_bhvTree; }
        inline const Resources::VulkanBuffer* GetVertexBuffer() const { return _vertexBuffer.get(); }
        inline const Resources::VulkanBuffer* GetIndexBuffer() const { return _indexBuffer.get(); }
    private:
        
        VulkanDevice& _device;

        std::vector<TracerUtils::Models::TracerMesh*> _models;
        std::unique_ptr<Resources::VulkanBuffer> _vertexBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _indexBuffer;
        std::unique_ptr<BHVTree> _bhvTree;
    };
    
} // namespace TracerCore
