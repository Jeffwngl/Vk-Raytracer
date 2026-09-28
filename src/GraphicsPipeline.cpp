#include "GraphicsPipeline.h"

#include "Utils.h"

namespace Vulkan {

void GraphicsPipeline::initialize(
    VkDevice device,
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo
) {
    this->device = device;

    createPipeline(
        vertFilePath,
        fragFilePath,
        configInfo
    );
}

VkPipeline GraphicsPipeline::getPipeline() const {
    return graphicsPipeline;
}

PipelineConfigInfo GraphicsPipeline::defaultPipelineConfigInfo() {
    PipelineConfigInfo config{};

    config.viewportCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    config.inputAssemblyCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    config.rasterizationCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f
    };

    config.multisampleCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE
    };

    config.colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    };

    config.colorBlendCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &config.colorBlendAttachment
    };

    config.depthStencilCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    // dynamicStateCI and renderingCI will be completed
    // inside createPipeline() where their referenced data
    // has a guaranteed lifetime.

    return config;
}

void GraphicsPipeline::createPipeline(
    const std::string& vertFilePath,
    const std::string& fragFilePath,
    const PipelineConfigInfo& configInfo
) {
    if (configInfo.pipelineLayout == VK_NULL_HANDLE) {
        throw std::runtime_error(
            "Graphics pipeline requires a valid pipeline layout."
        );
    }

    if (configInfo.colorFormat == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error(
            "Graphics pipeline requires a valid color format."
        );
    }

    auto vertShader = utils::readFile(vertFilePath);
    auto fragShader = utils::readFile(fragFilePath);

    VkShaderModule vertShaderModule = createShaderModule(vertShader);
    VkShaderModule fragShaderModule = createShaderModule(fragShader);

    VkPipelineShaderStageCreateInfo vertShaderStage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragShaderStage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main"
    };

    std::array<VkPipelineShaderStageCreateInfo, 2>shaderStages{
        vertShaderStage,
        fragShaderStage
    };

    // no traditional vertex buffer
    VkPipelineVertexInputStateCreateInfo vertexInputCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    // dynamic viewport and scissor
    std::array<VkDynamicState, 2> dynamicStates{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(
            dynamicStates.size()
        ),
        .pDynamicStates = dynamicStates.data()
    };

    // color blending
    VkPipelineColorBlendStateCreateInfo colorBlendCI = configInfo.colorBlendCI;
    colorBlendCI.pAttachments = &configInfo.colorBlendAttachment;

    // dynamic rendering
    VkPipelineRenderingCreateInfo renderingCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &configInfo.colorFormat,
        .depthAttachmentFormat = configInfo.depthFormat,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    // create pipeline
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        // required for dynamic rendering
        .pNext = &renderingCI,
        .stageCount = static_cast<uint32_t>(
            shaderStages.size()
        ),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputCI,
        .pInputAssemblyState = &configInfo.inputAssemblyCI,
        .pViewportState = &configInfo.viewportCI,
        .pRasterizationState = &configInfo.rasterizationCI,
        .pMultisampleState = &configInfo.multisampleCI,
        .pDepthStencilState = &configInfo.depthStencilCI,
        .pColorBlendState = &colorBlendCI,
        .pDynamicState = &dynamicStateCI,
        .layout = configInfo.pipelineLayout,
        // dynamic rendering means no VkRenderPass
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    utils::check(vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineCI,
        nullptr,
        &graphicsPipeline
    ));

    vkDestroyShaderModule(
        device,
        vertShaderModule,
        nullptr
    );

    vkDestroyShaderModule(
        device,
        fragShaderModule,
        nullptr
    );
}

VkShaderModule GraphicsPipeline::createShaderModule(
    const std::vector<char>& shader
) {
    if (shader.empty()) {
        throw std::runtime_error("Cannot create shader module from empty shader.");
    }

    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shader.size(),
        .pCode = reinterpret_cast<const uint32_t*>(shader.data())
    };

    VkShaderModule shaderModule{ VK_NULL_HANDLE };

    if (vkCreateShaderModule(
            device,
            &shaderModuleCI,
            nullptr,
            &shaderModule
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module.");
    }

    return shaderModule;
}

GraphicsPipeline::~GraphicsPipeline() {
    if (device != VK_NULL_HANDLE && graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(
            device,
            graphicsPipeline,
            nullptr
        );

        graphicsPipeline = VK_NULL_HANDLE;
    }
}

};