#include <array>

#include "ComputeDescriptorSet.h"
#include "Utils.h"

namespace Vulkan {

void ComputeDescriptorSet::initialize(
    Vulkan::VulkanCore& vkCore, 
    VkImageView outputImageView, 
    const Buffer& sceneObjectBuffer,
    const Buffer& materialBuffer
) {
    vulkanCore = &vkCore;

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet(outputImageView, sceneObjectBuffer, materialBuffer);
}

void ComputeDescriptorSet::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding outputImageBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutBinding sceneBufferBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutBinding materialBufferBinding{
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    std::array<VkDescriptorSetLayoutBinding, 3>bindings{
        outputImageBinding,
        sceneBufferBinding,
        materialBufferBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    utils::check(vkCreateDescriptorSetLayout(
        vulkanCore->getDevice().get(),
        &layoutCI,
        nullptr,
        &descriptorSetLayout
    ));
}

void ComputeDescriptorSet::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 2
        }
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    utils::check(vkCreateDescriptorPool(
        vulkanCore->getDevice().get(),
        &poolCI,
        nullptr,
        &descriptorPool
    ));
}

void ComputeDescriptorSet::createDescriptorSet(
    VkImageView outputImageView, 
    const Buffer& sceneObjectBuffer, 
    const Buffer& materialBuffer
) {
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    utils::check(vkAllocateDescriptorSets(
        vulkanCore->getDevice().get(),
        &allocInfo,
        &descriptorSet
    ));

    VkDescriptorImageInfo imageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = outputImageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkDescriptorBufferInfo sceneBufferInfo{
        .buffer = sceneObjectBuffer.get(),
        .offset = 0,
        .range = sceneObjectBuffer.getSize()
    };

    VkDescriptorBufferInfo materialBufferInfo{
        .buffer = materialBuffer.get(),
        .offset = 0,
        .range = materialBuffer.getSize()
    };

    VkWriteDescriptorSet outputImageWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo
    };

    VkWriteDescriptorSet sceneObjectBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &sceneBufferInfo
    };

    VkWriteDescriptorSet materialBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 2,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &materialBufferInfo
    };

    std::array<VkWriteDescriptorSet, 3>writes{
        outputImageWrite,
        sceneObjectBufferWrite,
        materialBufferWrite
    };

    vkUpdateDescriptorSets(
        vulkanCore->getDevice().get(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

VkDescriptorSet ComputeDescriptorSet::getDescriptorSet() const {
    return descriptorSet;
}

VkDescriptorPool ComputeDescriptorSet::getDescriptorPool() const {
    return descriptorPool;
}

VkDescriptorSetLayout ComputeDescriptorSet::getDescriptorSetLayout() const {
    return descriptorSetLayout;
}

ComputeDescriptorSet::~ComputeDescriptorSet() {
    if (!vulkanCore) {
        return;
    }

    VkDevice device = vulkanCore->getDevice().get();

    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(
            device,
            descriptorPool,
            nullptr
        );
    }

    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            nullptr
        );
    }
}

}