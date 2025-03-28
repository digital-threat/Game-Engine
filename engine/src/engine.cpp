#include <engine.h>

#include <vk_immediate.h>
#include <vk_mem_alloc.h>
#include <vk_images.h>
#include <vk_pipelines.h>
#include <vk_helpers.h>
#include <vk_buffers.h>
#include <input.h>
#include <utility.h>

#include <functional>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstring>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

void Engine::InitImGui()
{
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<u32>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &imguiPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(mWindow, true);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = mInstance;
    initInfo.PhysicalDevice = mPhysicalDevice;
    initInfo.Device = mDevice;
    initInfo.Queue = mGraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineRenderingCreateInfo = {};
    initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &mVkbSwapchain.image_format;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    ImGui_ImplVulkan_CreateFontsTexture();
}

void Engine::Run(Application* application)
{
    mApplication = application;
    FrameData frames[MAX_FRAMES_IN_FLIGHT]{};

    InitWindow();
    Input::Initialize(mWindow);
    InitVulkan(frames);
    InitImGui();
    MainLoop(frames);
    Cleanup();
}

void Engine::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
}

void Engine::InitVulkan(FrameData* frames)
{
    auto vkbInstance = CreateInstance();
    mInstance = vkbInstance.instance;
    CreateSurface();
    auto vkbPhysicalDevice = SelectPhysicalDevice(vkbInstance, mSurface);
    mPhysicalDevice = vkbPhysicalDevice.physical_device;
    auto vkbDevice = CreateDevice(vkbPhysicalDevice);
    CreateDispatchTable(vkbDevice);
    GetQueues(vkbDevice);
    CreateAllocator();
    CreateSwapchain(WIDTH, HEIGHT);
    InitSyncObjects(frames);
    CreateCommandObjects(vkbDevice, frames);
    InitDescriptors(frames);
    InitBuffers(frames);
    InitRaytracing();
    InitPipelines();
}

void Engine::MainLoop(FrameData* frames)
{
    u32 currentFrame = 0;

    TextureManager::Get().Awake();
    mApplication->Awake();

    double lastTime = 0;

    while (!glfwWindowShouldClose(mWindow))
    {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();

        if (mResizeRequested)
        {
            ResizeSwapchain();
        }

        mApplication->Update(deltaTime);
        mApplication->PhysicsUpdate(deltaTime);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        mApplication->Render();
        ImGui::Render();
        Render(frames[currentFrame % MAX_FRAMES_IN_FLIGHT]);
        currentFrame++;
    }

    vkDeviceWaitIdle(mDevice);

    mApplication->Destroy();
}

void Engine::Cleanup()
{
    glfwTerminate();
}

void Engine::ResizeSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    mVkbSwapchain.destroy_image_views(mSwapchainImageViews);

    CreateSwapchain(width, height);

    mResizeRequested = false;
}

vkb::Instance Engine::CreateInstance()
{
    vkb::InstanceBuilder builder;
    builder.set_app_name ("Application");
    builder.set_engine_name("No Engine");
    builder.request_validation_layers(enableValidationLayers);
    builder.use_default_debug_messenger();
    //builder.enable_layer("VK_LAYER_LUNARG_api_dump");
    builder.enable_layer("VK_LAYER_KHRONOS_validation");
    builder.require_api_version(1,3,0);

    auto instance = builder.build();

    if (!instance)
    {
        throw std::runtime_error("Failed to create Vulkan instance. Error: " + instance.error().message() + "\n");
    }

    return instance.value();
}

void Engine::CreateSurface()
{
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.\n");
    }
}

vkb::PhysicalDevice Engine::SelectPhysicalDevice(vkb::Instance& vkbInstance, VkSurfaceKHR vkSurface)
{
    VkPhysicalDeviceFeatures features{};
    features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features_12{};
    features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features_12.bufferDeviceAddress = true;
    features_12.descriptorIndexing = true;

    VkPhysicalDeviceVulkan13Features features_13{};
    features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features_13.dynamicRendering = true;
    features_13.synchronization2 = true;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeatures.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

    vkb::PhysicalDeviceSelector selector(vkbInstance);
    selector.set_surface(vkSurface);
    selector.set_minimum_version(1, 3);
    selector.set_required_features(features);
    selector.set_required_features_12(features_12);
    selector.set_required_features_13(features_13);
    selector.add_required_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    selector.add_required_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    selector.add_required_extension_features(rayTracingPipelineFeatures);
    selector.add_required_extension_features(accelerationStructureFeatures);

    auto physicalDevice = selector.select();

    if (!physicalDevice)
    {
        throw std::runtime_error("Failed to select physical device. Error: " + physicalDevice.error().message() + "\n");
    }

    return physicalDevice.value();
}

vkb::Device Engine::CreateDevice(vkb::PhysicalDevice& vkbPhysicalDevice)
{
    vkb::DeviceBuilder builder{ vkbPhysicalDevice };

    auto device = builder.build();

    if (!device)
    {
        throw std::runtime_error("Failed to create device. Error: " + device.error().message() + "\n");
    }

    mDevice = device.value().device;

    return device.value();
}

void Engine::CreateDispatchTable(vkb::Device vkbDevice)
{
    mVkbDispatchTable = vkbDevice.make_table();
}

void Engine::GetQueues(vkb::Device& device)
{
    auto graphicsQueue = device.get_queue(vkb::QueueType::graphics);

    if (!graphicsQueue)
    {
        throw std::runtime_error("Failed to get graphics queue. Error: " + graphicsQueue.error().message() + "\n");
    }

    mGraphicsQueue = graphicsQueue.value();
    mGraphicsQueueIndex = device.get_queue_index(vkb::QueueType::graphics).value();

    auto presentQueue = device.get_queue(vkb::QueueType::present);

    if (!presentQueue)
    {
        throw std::runtime_error("Failed to get present queue. Error: " + presentQueue.error().message() + "\n");
    }

    mPresentQueue = presentQueue.value();
    mPresentQueueIndex = device.get_queue_index(vkb::QueueType::present).value();
}

void Engine::CreateAllocator()
{
    VmaAllocatorCreateInfo info{};
    info.physicalDevice = mPhysicalDevice;
    info.device = mDevice;
    info.instance = mInstance;
    info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&info, &mAllocator);
}

void Engine::CreateSwapchain(u32 width, u32 height)
{
    vkb::SwapchainBuilder builder{ mPhysicalDevice, mDevice, mSurface, mGraphicsQueueIndex, mPresentQueueIndex };
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    builder.set_desired_extent(width, height);
    builder.set_old_swapchain(mVkbSwapchain);
    builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    auto swapchain = builder.build();

    if (!swapchain)
    {
        std::cout << swapchain.vk_result() << std::endl;
        throw std::runtime_error("Failed to create swapchain. Error: " + swapchain.error().message() + "\n");
    }

    destroy_swapchain(mVkbSwapchain);
    mVkbSwapchain = swapchain.value();

    mSwapchainImages = mVkbSwapchain.get_images().value();
    mSwapchainImageViews = mVkbSwapchain.get_image_views().value();

    mColorTarget.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    mColorTarget.extent = {width, height, 1};

    VkImageUsageFlags colorTargetUsage{};
    colorTargetUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    colorTargetUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    colorTargetUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    colorTargetUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo colorInfo = ImageCreateInfo(mColorTarget.format, colorTargetUsage, mColorTarget.extent);
    CreateImage(mAllocator, colorInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mColorTarget);
    VkImageViewCreateInfo colorViewInfo = ImageViewCreateInfo(mColorTarget.image, mColorTarget.format, VK_IMAGE_ASPECT_COLOR_BIT);
    if (vkCreateImageView(mDevice, &colorViewInfo, nullptr, &mColorTarget.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create color image view.");
    }

    std::array<VkFormat, 3> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    mDepthTarget.format = FindSupportedFormat(mPhysicalDevice, candidates, tiling, features);
    mDepthTarget.extent = {mVkbSwapchain.extent.width, mVkbSwapchain.extent.height, 1};

    VkImageUsageFlags depthTargetUsage{};
    depthTargetUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo depthInfo = ImageCreateInfo(mDepthTarget.format, depthTargetUsage, mDepthTarget.extent);
    CreateImage(mAllocator, depthInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthTarget);
    VkImageViewCreateInfo depthViewInfo = ImageViewCreateInfo(mDepthTarget.image, mDepthTarget.format, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (vkCreateImageView(mDevice, &depthViewInfo, nullptr, &mDepthTarget.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create depth image view.");
    }

    // Shadowmap

    mShadowmapTarget.format = mDepthTarget.format;
    mShadowmapTarget.extent = {1024, 1024, 1};

    VkImageUsageFlags shadowmapDepthTargetUsage{};
    shadowmapDepthTargetUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    shadowmapDepthTargetUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo shadowmapInfo = ImageCreateInfo(mShadowmapTarget.format, shadowmapDepthTargetUsage, mShadowmapTarget.extent);
    CreateImage(mAllocator, shadowmapInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mShadowmapTarget);
    VkImageViewCreateInfo shadowmapViewInfo = ImageViewCreateInfo(mShadowmapTarget.image, mShadowmapTarget.format, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (vkCreateImageView(mDevice, &shadowmapViewInfo, nullptr, &mShadowmapTarget.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create depth image view.");
    }
}

void Engine::InitPipelines()
{
    InitBackgroundPipeline();
    InitRasterizedPipeline();
    InitShadowmapPipeline();
    InitRaytracingPipeline();
}

void Engine::CreateCommandObjects(vkb::Device& device, FrameData* frames)
{
    vkb::Result<u32> graphicsQueueIndex = device.get_queue_index(vkb::QueueType::graphics);

    if (!graphicsQueueIndex)
    {
        throw std::runtime_error("Failed to get graphics queue family index. Error: " + graphicsQueueIndex.error().message() + "\n");
    }

    VkCommandPoolCreateInfo poolInfo = CommandPoolCreateInfo(graphicsQueueIndex.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Per frame
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &frames[i].commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool.");
        }

        VkCommandBufferAllocateInfo allocInfo = CommandBufferAllocateInfo(frames[i].commandPool, 1);

        if (vkAllocateCommandBuffers(mDevice, &allocInfo, &frames[i].mainCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers.");
        }
    }

    // Immediate
    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mImmediate.cmdPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }

    VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(mImmediate.cmdPool, 1);
    if (vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &mImmediate.cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }
}

void Engine::InitGlobalDescriptorAllocator()
{
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
    };

    mGlobalDescriptorAllocator.InitializePool(mDevice, 10, sizes);
}

void Engine::InitFrameDescriptorAllocators(FrameData* frames)
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        };

        frames[i].descriptorAllocator = DescriptorAllocator{};
        frames[i].descriptorAllocator.InitializePool(mDevice, 1000, sizes);
    }
}

void Engine::InitBackgroundDescriptorLayout()
{
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    mBackground.descriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_COMPUTE_BIT);
}

void Engine::InitSceneDescriptorLayout()
{
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    mSceneDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Engine::InitMaterialDescriptorLayout()
{
    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    mMaterialDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Engine::InitDescriptors(FrameData* frames)
{
    InitGlobalDescriptorAllocator();
    InitFrameDescriptorAllocators(frames);

    InitBackgroundDescriptorLayout();
    InitSceneDescriptorLayout();
    InitMaterialDescriptorLayout();

    mBackground.descriptorSet = mGlobalDescriptorAllocator.Allocate(mDevice, mBackground.descriptorLayout);

    DescriptorWriter writer;
    writer.WriteImage(0, mColorTarget.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(mDevice, mBackground.descriptorSet);
}

void Engine::InitBuffers(FrameData* frames)
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frames[i].sceneDataBuffer = CreateBuffer(mAllocator, sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
}

void Engine::InitSyncObjects(FrameData* frames)
{
    VkFenceCreateInfo fenceInfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreInfo = SemaphoreCreateInfo();

    // Per frame
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &frames[i].swapchainSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &frames[i].renderSemaphore) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &frames[i].renderFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects.");
        }
    }

    // Immediate
    if(vkCreateFence(mDevice, &fenceInfo, nullptr, &mImmediate.fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence.");
    }
}

void Engine::Render(FrameData& currentFrame)
{
    vkWaitForFences(mDevice, 1, &currentFrame.renderFence, VK_TRUE, UINT64_MAX);
    currentFrame.deletionQueue.Flush();
    currentFrame.descriptorAllocator.ClearDescriptors(mDevice);

    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mVkbSwapchain, UINT64_MAX, currentFrame.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        mResizeRequested = true;
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image.");
    }

    mRenderExtent.width = std::min(mVkbSwapchain.extent.width, mColorTarget.extent.width) * mApplication->mRenderContext.renderScale;
    mRenderExtent.height = std::min(mVkbSwapchain.extent.height, mColorTarget.extent.height) * mApplication->mRenderContext.renderScale;

    vkResetFences(mDevice, 1, &currentFrame.renderFence);
    vkResetCommandBuffer(currentFrame.mainCommandBuffer, 0);

    VkCommandBuffer cmd = currentFrame.mainCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    RenderBackground(cmd);

    TransitionImageLayout(cmd, mShadowmapTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    RenderShadowmap(cmd);
    TransitionImageLayout(cmd, mShadowmapTarget.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionImageLayout(cmd, mDepthTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    RenderRasterized(cmd, currentFrame);

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout(cmd, mSwapchainImages[imageIndex],VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyImage(cmd, mColorTarget.image, mSwapchainImages[imageIndex], mRenderExtent, mVkbSwapchain.extent);

    TransitionImageLayout(cmd, mSwapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    RenderImgui(cmd, mSwapchainImageViews[imageIndex]);


    TransitionImageLayout(cmd, mSwapchainImages[imageIndex],VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer.");
    }

    VkCommandBufferSubmitInfo cmdSubmitInfo = CommandBufferSubmitInfo(cmd);

    VkSemaphoreSubmitInfo waitInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, currentFrame.renderSemaphore);

    VkSubmitInfo2 submitInfo = SubmitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    if (vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, currentFrame.renderFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mVkbSwapchain.swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
        mResizeRequested = true;
        mFramebufferResized = false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Engine::UploadMesh(std::span<u32> indices, std::span<Vertex> vertices, GpuMesh& mesh)
{
    mesh.indexCount = indices.size();
    mesh.vertexCount = vertices.size();

    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(u32);

    VkBufferUsageFlags vertexBufferUsage{};
    vertexBufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    vertexBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vertexBufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    mesh.vertexBuffer = CreateBuffer(mAllocator, vertexBufferSize, vertexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo vertexAddressInfo{};
    vertexAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    vertexAddressInfo.buffer = mesh.vertexBuffer.buffer;
    mesh.vertexBufferAddress = vkGetBufferDeviceAddress(mDevice, &vertexAddressInfo);

    VkBufferUsageFlags indexBufferUsage{};
    indexBufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    indexBufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    mesh.indexBuffer = CreateBuffer(mAllocator, indexBufferSize, indexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo indexAddressInfo{};
    indexAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    indexAddressInfo.buffer = mesh.indexBuffer.buffer;
    mesh.indexBufferAddress = vkGetBufferDeviceAddress(mDevice, &indexAddressInfo);

    VulkanBuffer stagingBuffer = CreateBuffer(mAllocator, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = stagingBuffer.info.pMappedData;

    memcpy(data, vertices.data(), vertexBufferSize);
    memcpy(static_cast<char *>(data) + vertexBufferSize, indices.data(), indexBufferSize);

    auto func = [&](VkCommandBuffer pCmd)
    {
        VkBufferCopy vertexCopy{};
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(pCmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(pCmd, stagingBuffer.buffer, mesh.indexBuffer.buffer, 1, &indexCopy);
    };

    ImmediateSubmit(mDevice, mGraphicsQueue, mImmediate, func);

    DestroyBuffer(mAllocator, stagingBuffer);
}
