#include "ComputeDescriptorSet.h"
#include "Utils.h"

void ComputeDescriptorSet::initialize(VulkanCore& vkCore, VkImageView outputImageView) {
    vulkanCore = &vkCore;

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet(outputImageView);
}

void ComputeDescriptorSet::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding outputImageBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &outputImageBinding
    };

    utils::check(vkCreateDescriptorSetLayout(
        vulkanCore->getDevice(),
        &layoutCI,
        nullptr,
        &descriptorSetLayout
    ));
}

void ComputeDescriptorSet::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    utils::check(vkCreateDescriptorPool(
        vulkanCore->getDevice(),
        &poolCI,
        nullptr,
        &descriptorPool
    ));
}

void ComputeDescriptorSet::createDescriptorSet(VkImageView outputImageView) {
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

    VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imageInfo
    };

    vkUpdateDescriptorSets(
        vulkanCore->getDevice(),
        1,
        &descriptorWrite,
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
