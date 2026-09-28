#include "GraphicsDescriptorSet.h"

#include "Utils.h"

namespace Vulkan {

void GraphicsDescriptorSet::initialize(
    VulkanCore& vkCore,
    Buffer& triangleObjectBuffer
) {
    vulkanCore = &vkCore;

    createDescriptorSetLayout();
    createDescriptorSet(triangleObjectBuffer);
}

VkDescriptorSet GraphicsDescriptorSet::getDescriptorSet() const {
    return descriptorSet;
}

VkDescriptorSetLayout GraphicsDescriptorSet::getDescriptorSetLayout() const {
    return descriptorSetLayout;
}

void GraphicsDescriptorSet::createDescriptorSet(const Buffer& triangleObjectBuffer) {
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = vulkanCore->getDescriptorPool(),
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };

    utils::check(
        vkAllocateDescriptorSets(
            vulkanCore->getDevice().get(),
            &allocInfo,
            &descriptorSet
        )
    );

    VkDescriptorBufferInfo bufferInfo{
        .buffer = triangleObjectBuffer.get(),
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferInfo
    };

    vkUpdateDescriptorSets(
        vulkanCore->getDevice().get(),
        1,
        &write,
        0,
        nullptr
    );
}

void GraphicsDescriptorSet::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding triangleBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &triangleBinding
    };

    utils::check(
        vkCreateDescriptorSetLayout(
            vulkanCore->getDevice().get(),
            &layoutInfo,
            nullptr,
            &descriptorSetLayout
        )
    );
}

GraphicsDescriptorSet::~GraphicsDescriptorSet() {
    if (!vulkanCore) {
        return;
    }

    VkDevice device = vulkanCore->getDevice().get();

    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            nullptr
        );
    }
}

}