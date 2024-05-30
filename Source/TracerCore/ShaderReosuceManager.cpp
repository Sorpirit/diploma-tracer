

#include "ShaderReosuceManager.hpp"

namespace TracerCore
{
    ShaderReosuceManager::ShaderReosuceManager(VulkanDevice &device)
        : _device(device)
    {
    }

    ShaderReosuceManager::~ShaderReosuceManager()
    {
    }

    void ShaderReosuceManager::CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding* layoutBindings, int bindingCount, VkDescriptorSetLayout& descriptorSetLayout) 
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindingCount;
        layoutInfo.pBindings = layoutBindings;

        if (vkCreateDescriptorSetLayout(_device.GetVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }


    void ShaderReosuceManager::CreateDescriptorPool(const VkDescriptorPoolSize* poolSizes, int poolSizeCount, int maxSets, VkDescriptorPool& descriptorPool)
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizeCount;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = static_cast<uint32_t>(maxSets);

        if (vkCreateDescriptorPool(_device.GetVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void ShaderReosuceManager::CreateDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout, int setCount, std::vector<VkDescriptorSet>& descriptorSets)
    {
        std::vector<VkDescriptorSetLayout> layouts(setCount, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(setCount);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(setCount);
        
        if (vkAllocateDescriptorSets(_device.GetVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
    }

    void ShaderReosuceManager::UploadBuffer(const std::vector<VkDescriptorSet>& descriptorSets, int dtsBinding, VkDescriptorType descriptorType, 
            const Resources::VulkanBuffer* buffer)
    {
        for (size_t i = 0; i < descriptorSets.size(); i++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = buffer->GetSize();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = dtsBinding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = descriptorType;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void ShaderReosuceManager::UploadTexture(const std::vector<VkDescriptorSet>& descriptorSets, int dtsBinding, VkImageLayout targetLayout, VkDescriptorType descriptorType, const Resources::Texture2D* texture)
    {
        for (size_t i = 0; i < descriptorSets.size(); i++) 
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = targetLayout;
            imageInfo.imageView = texture->GetImageView();
            imageInfo.sampler = texture->GetSampler();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = dtsBinding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = descriptorType;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = nullptr;
            descriptorWrite.pImageInfo = &imageInfo; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(_device.GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }
} // namespace TracerCore
