#pragma once

#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace TracerCore {

    struct Vertex {
        public:
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> GetBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    class Model {
        public:
            Model(VulkanDevice& _device, const std::vector<Vertex>& vertices);
            ~Model();

            Model(const Model&) = delete;
            Model operator=(const Model&) = delete;

            void Bind(VkCommandBuffer commandBuffer);
            void Draw(VkCommandBuffer commandBuffer);
        private:
            void CreateVertexBuffer(const std::vector<Vertex>& vertices);

            VulkanDevice& _device;
            VkBuffer _vertexBuffer;
            VkDeviceMemory _vertexBufferMemory;
            uint32_t _vertexCount;
    };

}