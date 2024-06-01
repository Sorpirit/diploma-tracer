#include "TracerScene.hpp"

#include "AccelerationStructures/BHVTree.hpp"
#include "AccelerationStructures/KdTree.hpp"

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

    void TracerScene::BuildScene()
    {
        std::vector<TracerUtils::Models::TracerVertex> vertecies;
        std::vector<uint32_t> indices;

        for (const auto model : _models)
        {
            vertecies.insert(vertecies.end(), model->Vertices.begin(), model->Vertices.end());
            indices.insert(indices.end(), model->Indices.begin(), model->Indices.end());
        }

        for (const auto vertex : vertecies)
        {
            _aabbMin = glm::min(_aabbMin, vertex.Position);
            _aabbMax = glm::max(_aabbMax, vertex.Position);
        }

        _models.clear();

        //_accBHVStructure = std::make_unique<AccelerationStructures::BHVTree>(_device, vertecies, indices);
        _accKdTreeStructure = std::make_unique<AccelerationStructures::KdTree>(_device, vertecies, indices);

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

    void TracerScene::AttachSceneGeometry(const ShaderReosuceManager &resourceManager, std::vector<VkDescriptorSet> &descriptosSets) const
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
    }

} // namespace TacerCore


