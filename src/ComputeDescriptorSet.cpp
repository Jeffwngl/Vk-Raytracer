#include <array>

#include "ComputeDescriptorSet.h"
#include "Utils.h"

namespace Vulkan {

void ComputeDescriptorSet::initialize(
    Vulkan::VulkanCore& vkCore, 
    VkImageView outputImageView, 
    VkImageView accumulatedImageView,
    const Buffer& sphereObjectBuffer,
    const Buffer& triangleObjectBuffer,
    const Buffer& materialBuffer,
    const Buffer& bvhBuffer
) {
    vulkanCore = &vkCore;

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet(
        outputImageView, 
        accumulatedImageView,
        sphereObjectBuffer, 
        triangleObjectBuffer,
        materialBuffer,
        bvhBuffer
    );
}

void ComputeDescriptorSet::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding outputImageBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutBinding accumulatedImageBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutBinding sphereBufferBinding{
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };


    VkDescriptorSetLayoutBinding triangleBufferBinding{
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };


    VkDescriptorSetLayoutBinding materialBufferBinding{
        .binding = 4,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    VkDescriptorSetLayoutBinding bvhBufferBinding{
        .binding = 5,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
    };

    std::array<VkDescriptorSetLayoutBinding, 6>bindings{
        outputImageBinding,
        accumulatedImageBinding,
        sphereBufferBinding,
        triangleBufferBinding,
        materialBufferBinding,
        bvhBufferBinding
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
            .descriptorCount = 2
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 4
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
    VkImageView accumulatedImageView,
    const Buffer& sphereObjectBuffer, 
    const Buffer& triangleObjectBuffer,
    const Buffer& materialBuffer,
    const Buffer& bvhBuffer
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

    VkDescriptorImageInfo outputImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = outputImageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkDescriptorImageInfo accumualtedImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = accumulatedImageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkDescriptorBufferInfo sphereBufferInfo{
        .buffer = sphereObjectBuffer.get(),
        .offset = 0,
        .range = sphereObjectBuffer.getSize()
    };

    VkDescriptorBufferInfo triangleBufferInfo{
        .buffer = triangleObjectBuffer.get(),
        .offset = 0,
        .range = triangleObjectBuffer.getSize()
    };

    VkDescriptorBufferInfo materialBufferInfo{
        .buffer = materialBuffer.get(),
        .offset = 0,
        .range = materialBuffer.getSize()
    };

    VkDescriptorBufferInfo bvhBufferInfo{
        .buffer = bvhBuffer.get(),
        .offset = 0,
        .range = bvhBuffer.getSize()
    };

    VkWriteDescriptorSet outputImageWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &outputImageInfo
    };

    VkWriteDescriptorSet accumulatedImageWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &accumualtedImageInfo
    };

    VkWriteDescriptorSet sphereObjectBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 2,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &sphereBufferInfo
    };

    VkWriteDescriptorSet triangleObjectBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 3,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &triangleBufferInfo
    };

    VkWriteDescriptorSet materialBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 4,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &materialBufferInfo
    };

    VkWriteDescriptorSet bvhBufferWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 5,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bvhBufferInfo
    };

    std::array<VkWriteDescriptorSet, 6>writes{
        outputImageWrite,
        accumulatedImageWrite,
        sphereObjectBufferWrite,
        triangleObjectBufferWrite,
        materialBufferWrite,
        bvhBufferWrite
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