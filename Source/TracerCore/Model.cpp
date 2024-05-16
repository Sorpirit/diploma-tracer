#include "Model.hpp"

#include <cassert>

namespace TracerCore
{
    std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescription() {
        std::vector<VkVertexInputBindingDescription> attributeDescriptions{};
        attributeDescriptions.resize(1);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].stride = sizeof(Vertex);
        attributeDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return attributeDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.resize(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }

    Model::Model(VulkanDevice& device, const std::vector<Vertex>& vertices) : _device{device}
    {
        CreateVertexBuffer(vertices);
    }

    Model::~Model()
    {
        vkDestroyBuffer(_device.GetVkDevice(), _vertexBuffer, nullptr);
        vkFreeMemory(_device.GetVkDevice(), _vertexBufferMemory, nullptr);
    }

    void Model::Bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {_vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void Model::Draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, _vertexCount, 1, 0, 0);
    }

    void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
    {
        _vertexCount = static_cast<uint32_t>(vertices.size());
        assert(_vertexCount >= 3 && "Vertex count must be at least 3");

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        // VkBuffer stagingBuffer;
        // VkDeviceMemory stagingBufferMemory;
        // _device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _vertexBuffer, _vertexBufferMemory);

        // void* data;
        // vkMapMemory(_device.GetVkDevice(), _vertexBufferMemory, 0, bufferSize, 0, &data);
        // memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        // vkUnmapMemory(_device.GetVkDevice(), _vertexBufferMemory);

        // _device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
        // _device.CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

        // vkDestroyBuffer(_device.GetVkDevice(), stagingBuffer, nullptr);
    }
}