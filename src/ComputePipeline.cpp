#include "ComputePipeline.h"
#include "Utils.h"

namespace Vulkan {

void ComputePipeline::initialize(VulkanCore& vkCore, std::string& path, VkDescriptorSetLayout descriptorSetLayout) {
    vulkanCore = &vkCore;

    auto computeShader = utils::readFile(path); // readfile

    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = computeShader.size(),
        .pCode = reinterpret_cast<const uint32_t*>(computeShader.data())
    };

    VkShaderModule shaderModule;

    utils::check(vkCreateShaderModule(
        vulkanCore->getDevice().get(),
        &shaderModuleCI,
        VK_NULL_HANDLE,
        &shaderModule
    ));

    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(uint32_t)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    utils::check(vkCreatePipelineLayout(
        vulkanCore->getDevice().get(),
        &pipelineLayoutCI,
        VK_NULL_HANDLE,
        &pipelineLayout
    ));

    VkPipelineShaderStageCreateInfo shaderStageCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = shaderModule,
        .pName = "main"
    };

    VkComputePipelineCreateInfo computePipelineCI{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .layout = pipelineLayout,
        .stage = shaderStageCI
    };

    utils::check(vkCreateComputePipelines(
        vulkanCore->getDevice().get(),
        VK_NULL_HANDLE,
        1,
        &computePipelineCI,
        VK_NULL_HANDLE,
        &pipeline
    ));

    vkDestroyShaderModule(vulkanCore->getDevice().get(), shaderModule, VK_NULL_HANDLE);
}

VkPipeline ComputePipeline::getPipeline() const {
    return pipeline;
}

VkPipelineLayout ComputePipeline::getPipelineLayout() const {
    return pipelineLayout;
}

ComputePipeline::~ComputePipeline() {
    if (!vulkanCore) {
        return;
    };

    VkDevice device = vulkanCore->getDevice().get();

    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}

}