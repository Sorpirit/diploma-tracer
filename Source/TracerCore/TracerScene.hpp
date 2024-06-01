#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "VulkanDevice.hpp"
#include "Resources/VulkanBuffer.hpp"
#include "Models/TracerMesh.hpp"
#include "ShaderReosuceManager.hpp"
#include "AccelerationStructures/AccelerationStructure.hpp"

namespace TracerCore
{
    enum class AccStructureType
    {
        AccStructure_None,
        AccStructure_BVH,
        AccStructure_KdTree
    };

    class TracerScene
    {
    public:
        TracerScene(VulkanDevice& device);
        ~TracerScene();

        TracerScene(const TracerScene&) = delete;
        TracerScene &operator=(const TracerScene&) = delete;

        void AddModel(TracerUtils::Models::TracerMesh& model);
        void BuildScene();

        inline const AccelerationStructures::AccelerationStructure& GetAccelerationStructure() const { return *_accStructure; }
        inline const Resources::VulkanBuffer* GetVertexBuffer() const { return _vertexBuffer.get(); }
        inline const Resources::VulkanBuffer* GetIndexBuffer() const { return _accStructure == nullptr ? _indexBuffer.get() : _accStructure->GetIndicesBuffer(); }
    
        inline const glm::vec3& GetAABBMin() const { return _aabbMin; }
        inline const glm::vec3& GetAABBMax() const { return _aabbMax; }
    
        void AttachSceneGeometry(const ShaderReosuceManager& resourceManager, std::vector<VkDescriptorSet>& descriptosSets) const;

        inline void SetAccMode(AccStructureType accType) {
            switch (accType)
            {
            case AccStructureType::AccStructure_BVH:
                _accStructure = _accBHVStructure.get();
                break;
            case AccStructureType::AccStructure_KdTree:
                _accStructure = _accKdTreeStructure.get();
                break;
            default:
                _accStructure = nullptr;
                break;
            }
        } 

    private:
        
        VulkanDevice& _device;

        std::vector<TracerUtils::Models::TracerMesh*> _models;
        std::unique_ptr<Resources::VulkanBuffer> _vertexBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _indexBuffer;

        AccelerationStructures::AccelerationStructure* _accStructure;

        std::unique_ptr<AccelerationStructures::AccelerationStructure> _accBHVStructure;
        std::unique_ptr<AccelerationStructures::AccelerationStructure> _accKdTreeStructure;

        glm::vec3 _aabbMin = glm::vec3(FLT_MAX);
        glm::vec3 _aabbMax = glm::vec3(-FLT_MAX);
    };
    
} // namespace TracerCore
