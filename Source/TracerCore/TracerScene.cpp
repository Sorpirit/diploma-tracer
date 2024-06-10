#include "TracerScene.hpp"

#include "AccelerationStructures/BHVTree.hpp"
#include "AccelerationStructures/KdTree.hpp"

#include <tracy/Tracy.hpp>

#include "../TracerUtils/Math/RandomHelper.hpp"

namespace TracerCore
{

    TracerScene::TracerScene(VulkanDevice &device) : _device(device)
    {
    }

    TracerScene::~TracerScene()
    {
    }

    void TracerScene::AddModel(TracerUtils::Models::TracerMesh& model)
    {
        _models.push_back(&model);
    }

    void TracerScene::BuildMaterials(const MaterialsSettings& materialsSettings)
    {
        _materials.clear();

        Material groundMaterial;
        groundMaterial.albedo = materialsSettings.groundAlbedo;
        groundMaterial.fuzz = 1.0f;
        _materials.push_back(groundMaterial);

        for (const auto& model : _models)
        {
            for (auto& part : model->Parts)
            {
                Material material;

                material.albedo = !materialsSettings.UseRandomMaterials ? materialsSettings.meshAlbedo : glm::vec3(
                        TracerUtils::Math::TracerRandom::RandomFloat(0.0f, 1.0f), 
                        TracerUtils::Math::TracerRandom::RandomFloat(0.0f, 1.0f), 
                        TracerUtils::Math::TracerRandom::RandomFloat(0.0f, 1.0f));
                material.fuzz = !materialsSettings.UseRandomMaterials ? materialsSettings.meshFuzz : 
                    TracerUtils::Math::TracerRandom::RandomFloat(0.0, 1.0f) > 0.5f ? TracerUtils::Math::TracerRandom::RandomFloat(0.05f, 0.9f) : 1.0f;
                int materialIndex = _materials.size();
                _materials.push_back(material);

                for (auto& vertex : part.Vertices)
                {
                    vertex.MaterialFlag = materialIndex;
                }
            }
        }

        VkDeviceSize materialsSize = sizeof(Material) * _materials.size();
        _materialsBuffer = Resources::VulkanBuffer::CreateBuffer(_device, materialsSize, 
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        void *data;
        _materialsBuffer->MapMemory(materialsSize, 0, &data);
        memcpy(data, _materials.data(), materialsSize);
        _materialsBuffer->UnmapMemory();

        _materials.clear();
    }

    void TracerScene::BuildScene(AccStructureType accType, AccHeruishitcType accHeruishitcType)
    {
        ZoneScoped;
        std::vector<TracerUtils::Models::TracerVertex> vertecies;
        std::vector<uint32_t> indices;

        _vertexBuffer = nullptr;
        _indexBuffer = nullptr;

        for (const auto model : _models)
        {
            for (const auto part : model->Parts)
            {
                uint32_t partOffset = vertecies.size();
                vertecies.insert(vertecies.end(), part.Vertices.begin(), part.Vertices.end());

                for (size_t i = 0; i < part.Indices.size(); i++)
                {
                    indices.push_back(part.Indices[i] + partOffset);
                }
            }
        }

        for (const auto vertex : vertecies)
        {
            _aabbMin = glm::min(_aabbMin, vertex.Position);
            _aabbMax = glm::max(_aabbMax, vertex.Position);
        }

        _models.clear();

        _accStructure = nullptr;
        switch (accType)
        {
            case AccStructureType::AccStructure_BVH:
                _accStructure = std::make_unique<AccelerationStructures::BHVTree>(_device, accHeruishitcType, vertecies, indices);
                break;
            case AccStructureType::AccStructure_KdTree:
                auto kdTree = std::make_unique<AccelerationStructures::KdTree>(_device, accHeruishitcType, vertecies, indices);
                _aabbMin = kdTree->GetAABBMin();
                _aabbMax = kdTree->GetAABBMax();
                _accStructure = std::move(kdTree);
                break;
        }

        //Upload scene data to GPU
        VkDeviceSize verteciesSize = sizeof(TracerUtils::Models::TracerVertex) * vertecies.size();
        VkDeviceSize indicesSize = sizeof(uint32_t) * indices.size();

        _vertexBuffer = Resources::VulkanBuffer::CreateBuffer(_device, verteciesSize, 
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
        
        _indexBuffer = Resources::VulkanBuffer::CreateBuffer(_device, indicesSize, 
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

        void *data;
        _vertexBuffer->MapMemory(verteciesSize, 0, &data);
        memcpy(data, vertecies.data(), verteciesSize);
        _vertexBuffer->UnmapMemory();

        _indexBuffer->MapMemory(indicesSize, 0, &data);
        memcpy(data, indices.data(), indicesSize);
        _indexBuffer->UnmapMemory();
    }

    void TracerScene::AttachSceneGeometry(const ShaderReosuceManager &resourceManager, const std::vector<VkDescriptorSet> &descriptosSets) const
    {
        resourceManager.UploadBuffer(descriptosSets, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _vertexBuffer.get());
        
        if(_accStructure != nullptr)
        {
            resourceManager.UploadBuffer(descriptosSets, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _accStructure->GetIndicesBuffer());
            resourceManager.UploadBuffer(descriptosSets, 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _accStructure->GetNodesBuffer());
        }
        else
        {
            resourceManager.UploadBuffer(descriptosSets, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _indexBuffer.get());
        }

        resourceManager.UploadBuffer(descriptosSets, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _materialsBuffer.get());
    }

} // namespace TacerCore


