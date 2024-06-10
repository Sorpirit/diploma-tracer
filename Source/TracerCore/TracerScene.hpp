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

    enum class AccHeruishitcType
    {
        AccHeruishitc_Primitive,
        AccHeruishitc_SAH
    };

    struct Material
    {
        alignas(16) glm::vec3 albedo;
        alignas(4) float fuzz;
    };

    struct MaterialsSettings
    {
        glm::vec3 groundAlbedo = glm::vec3(0.5f);
        bool UseRandomMaterials = true;
        glm::vec3 meshAlbedo = glm::vec3(0.5f);
        float meshFuzz = 0.0f;
    };

    class TracerScene
    {
    public:
        TracerScene(VulkanDevice& device);
        ~TracerScene();

        TracerScene(const TracerScene&) = delete;
        TracerScene &operator=(const TracerScene&) = delete;

        void AddModel(TracerUtils::Models::TracerMesh& model);
        void BuildMaterials(const MaterialsSettings& materialsSettings);
        void BuildScene(AccStructureType accType, AccHeruishitcType accHeruishitcType);

        inline const AccelerationStructures::AccelerationStructure& GetAccelerationStructure() const { return *_accStructure; }
        inline const Resources::VulkanBuffer* GetVertexBuffer() const { return _vertexBuffer.get(); }
        inline const Resources::VulkanBuffer* GetIndexBuffer() const { return _accStructure == nullptr ? _indexBuffer.get() : _accStructure->GetIndicesBuffer(); }
    
        inline const glm::vec3& GetAABBMin() const { return _aabbMin; }
        inline const glm::vec3& GetAABBMax() const { return _aabbMax; }
        inline const uint32_t GetIndeciesCount() const { return _accStructure == nullptr ? 0 : _accStructure->GetIndeciesCount(); }
    
        void AttachSceneGeometry(const ShaderReosuceManager& resourceManager, const std::vector<VkDescriptorSet>& descriptosSets) const;

    private:
        
        VulkanDevice& _device;

        std::vector<TracerUtils::Models::TracerMesh*> _models;
        std::vector<Material> _materials;

        std::unique_ptr<Resources::VulkanBuffer> _vertexBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _indexBuffer;
        std::unique_ptr<Resources::VulkanBuffer> _materialsBuffer;

        std::unique_ptr<AccelerationStructures::AccelerationStructure> _accStructure;

        glm::vec3 _aabbMin = glm::vec3(FLT_MAX);
        glm::vec3 _aabbMax = glm::vec3(-FLT_MAX);
    };
    
} // namespace TracerCore
