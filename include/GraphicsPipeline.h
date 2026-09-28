#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct PipelineConfigInfo {
    VkPipelineViewportStateCreateInfo viewportCI{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
    VkPipelineRasterizationStateCreateInfo rasterizationCI{};
    VkPipelineRenderingCreateInfo renderingCI{};
    VkPipelineMultisampleStateCreateInfo multisampleCI{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendCI{};
    VkPipelineDepthStencilStateCreateInfo depthStencilCI{};

    VkPipelineDynamicStateCreateInfo dynamicStateCI{};

    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

    VkFormat colorFormat{VK_FORMAT_UNDEFINED};
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};
};

namespace Vulkan {

struct GraphicsPushConstants {
    glm::mat4 viewProjection;
};

class GraphicsPipeline {
public:
    GraphicsPipeline() = default;
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    void initialize(
        VkDevice device,
        const std::string& vertFilePath,
        const std::string& fragFilePath,
        const PipelineConfigInfo& configInfo
    );

    static PipelineConfigInfo defaultPipelineConfigInfo();

    VkPipeline getPipeline() const;

private:
    void createPipeline(
        const std::string& vertFilePath,
        const std::string& fragFilePath,
        const PipelineConfigInfo& configInfo
    );

    VkShaderModule createShaderModule(const std::vector<char>& shader);

private:
    VkDevice device{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
};

}