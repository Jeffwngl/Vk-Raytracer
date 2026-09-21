#include "ImGuiLayer.h"

void ImGuiLayer::initialize(Vulkan::VulkanCore& vkCore) {
    vulkanCore = &vkCore;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForVulkan(
        vkCore.getWindow()
    );

    VkFormat swapchainFormat = vkCore.getSwapchain().getFormat();

    ImGui_ImplVulkan_InitInfo initInfo{};

    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = vkCore.getInstance();
    initInfo.PhysicalDevice = vkCore.getDevice().getPhysicalDevice();
    initInfo.Device = vkCore.getDevice().get();
    initInfo.QueueFamily = vkCore.getDevice().getQueueFamily();
    initInfo.Queue = vkCore.getDevice().getQueue().get();

    // let ImGui make its own descriptor pool
    initInfo.DescriptorPoolSize = 32;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount =
        static_cast<uint32_t>(
            vkCore.getSwapchain().getImages().size()
        );
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineRenderingCreateInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchainFormat
    };

    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = renderingInfo;

    ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiLayer::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    return;
}

void ImGuiLayer::beginFrame() {
    // (After event loop)
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::build(float fps) {
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("Raytracer");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::End();
}

void ImGuiLayer::render(VkCommandBuffer commandBuffer) {
    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    // (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
}

void ImGuiLayer::cleanUp() {
    if (vulkanCore == nullptr) {
        return;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    vulkanCore = nullptr;
}