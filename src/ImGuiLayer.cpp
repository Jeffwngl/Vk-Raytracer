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

bool ImGuiLayer::build(
    RenderSettings& settings, 
    Camera& camera, 
    uint32_t accumulatedFrames
) {
    bool changed = false;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(360, 350));
    ImGui::SetNextWindowPos(
        ImVec2(10.0f + 360.0f, 10.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)
    );

    ImGui::Begin("Settings");

    ImGui::Spacing();

    ImGui::Text("Help");
    ImGui::Separator();
    ImGui::Text("- `WASD` to move.");
    ImGui::Text("- Press `T` to toggle mouse.");
    ImGui::Spacing();

    ImGui::Text("Scene");
    ImGui::Separator();
    
    ImGui::Spacing();

    ImGui::Text("Ray tracing");
    ImGui::Separator();
    bool accumulate = settings.accumulateRays != 0;
    if (ImGui::Checkbox("Accumulate Rays", &accumulate)) {
        settings.accumulateRays = accumulate ? 1u : 0u;
    }
    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderScalar(
        "Samples (Per Pixel)",
        ImGuiDataType_U32,
        &settings.samplesPerPixel,
        &sliderMin,
        &sliderMax
    );
    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderScalar(
        "Bounces",
        ImGuiDataType_U32,
        &settings.maxBounces,
        &sliderMin,
        &sliderMax
    );
    ImGui::Spacing();

    ImGui::Text("Camera");
    ImGui::Separator();
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat(
        "FOV",
        &camera.getFov(),
        1.0f,
        100.0f
    )) {
        changed = true;
    }
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat(
        "Focus Distance",
        &camera.getFocusDist(),
        1.0f,
        50.0f
    )) {
        changed = true;
    }
    ImGui::SetNextItemWidth(200.0f);
    if (ImGui::SliderFloat(
        "Defocus Angle",
        &camera.getDefocusAngle(),
        0.0f,
        5.0f,
        "%.2f deg"
    )) {
        changed = true;
    }
    ImGui::Spacing();

    ImGui::End();

    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x - 10.0f, 10.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)
    );
    ImGui::SetNextWindowSize(
        ImVec2(200.0f, 100.0f),
        ImGuiCond_Always
    );
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::Begin("Stats", nullptr, flags);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("Accumulated Frames: %u", accumulatedFrames);
    ImGui::End();

    return changed;
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