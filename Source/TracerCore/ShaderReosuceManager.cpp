

#include "ShaderReosuceManager.hpp"

namespace TracerCore
{
    ShaderReosuceManager::ShaderReosuceManager(VulkanDevice &device, int maxImagesInFlight)
        : _device(device), _maxImagesInFlight(maxImagesInFlight)
    {
        Init();
    }

    ShaderReosuceManager::~ShaderReosuceManager()
    {
        vkDestroyDescriptorPool(_device.GetVkDevice(), _descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(_device.GetVkDevice(), _descriptorSetLayout, nullptr);
    }

    void ShaderReosuceManager::Init()
    {
        CreateDescriptorPool();
        CreateDescriptorSetLayout(_descriptorSetLayout);
        CreateDescriptorSets(_descriptorSetLayout);
    }

    void ShaderReosuceManager::CreateDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout) 
    {
        // VkDescriptorSetLayoutBinding uboLayoutBinding{};
        // uboLayoutBinding.binding = 0;
        // uboLayoutBinding.descriptorCount = 1;
        // uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // uboLayoutBinding.pImmutableSamplers = nullptr;
        // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(_device.GetVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }


    void ShaderReosuceManager::CreateDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 1> poolSizes{};
        // poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // poolSizes[0].descriptorCount = static_cast<uint32_t>(_maxImagesInFlight);
        // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // poolSizes[1].descriptorCount = static_cast<uint32_t>(_maxImagesInFlight);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(_maxImagesInFlight);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(_maxImagesInFlight);

        if (vkCreateDescriptorPool(_device.GetVkDevice(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void ShaderReosuceManager::CreateDescriptorSets(VkDescriptorSetLayout& descriptorSetLayout)
    {
        std::vector<VkDescriptorSetLayout> layouts(_maxImagesInFlight, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_maxImagesInFlight);
        allocInfo.pSetLayouts = layouts.data();

        _descriptorSets.resize(_maxImagesInFlight);
        
        if (vkAllocateDescriptorSets(_device.GetVkDevice(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    void ShaderReosuceManager::UploadBuffer(VkBuffer buffer, VkDeviceSize stride)
    {
        for (size_t i = 0; i < _maxImagesInFlight; i++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = stride;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void ShaderReosuceManager::UploadTexture(const Texture2D* texture)
    {
        for (size_t i = 0; i < _maxImagesInFlight; i++) 
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->GetImageView();
            imageInfo.sampler = texture->GetSampler();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = nullptr;
            descriptorWrite.pImageInfo = &imageInfo; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void ShaderReosuceManager::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int imageIndex)
    {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &_descriptorSets[imageIndex], 0, nullptr);
    }

} // namespace TracerCore
