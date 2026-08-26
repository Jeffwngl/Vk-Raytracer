#include <array>

#include "ComputeDescriptorSet.h"
#include "Utils.h"

namespace Vulkan {

void ComputeDescriptorSet::initialize(Vulkan::VulkanCore& vkCore, VkImageView outputImageView, const Buffer& sceneObjectBuffer) {
    vulkanCore = &vkCore;

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet(outputImageView, sceneObjectBuffer);
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

    std::array<VkDescriptorSetLayoutBinding, 2>bindings{
        outputImageBinding,
        sceneBufferBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    utils::check(vkCreateDescriptorSetLayout(
        vulkanCore->getDevice(),
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
            .descriptorCount = 1
        }
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    utils::check(vkCreateDescriptorPool(
        vulkanCore->getDevice(),
        &poolCI,
        nullptr,
        &descriptorPool
    ));
}

void ComputeDescriptorSet::createDescriptorSet(VkImageView outputImageView, const Buffer& sceneObjectBuffer) {
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    utils::check(vkAllocateDescriptorSets(
        vulkanCore->getDevice(),
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

    std::array<VkWriteDescriptorSet, 2>writes{
        outputImageWrite,
        sceneObjectBufferWrite
    };

    vkUpdateDescriptorSets(
        vulkanCore->getDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0,
        nullptr
    );
}

VkDescriptorSet ComputeDescriptorSet::getDescriptorSet() {
    return descriptorSet;
}

VkDescriptorSetLayout ComputeDescriptorSet::getDescriptorSetLayout() {
    return descriptorSetLayout;
}

ComputeDescriptorSet::~ComputeDescriptorSet() {
    if (!vulkanCore) {
        return;
    }

    VkDevice device = vulkanCore->getDevice();

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