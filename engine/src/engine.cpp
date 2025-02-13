#include "engine.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstring>
#include <renderer_vk_utility.h>
#include <vk_mem_alloc.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "renderer_vk_images.h"
#include "renderer_vk_pipelines.h"
#include "renderer_vk_structures.h"
#include "renderer_vk_buffers.h"

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

    ImGui_ImplGlfw_InitForVulkan(window, true);
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

void Engine::ResizeSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    mVkbSwapchain.destroy_image_views(mSwapchainImageViews);

    CreateSwapchain(width, height);

    mResizeRequested = false;
}

void Engine::CreateInstance()
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

    mVkbInstance = instance.value();
    mInstance = mVkbInstance.instance;
}

void Engine::CreateSurface()
{
    if (glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.\n");
    }
}

void Engine::SelectPhysicalDevice()
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

    vkb::PhysicalDeviceSelector selector(mVkbInstance);
    selector.set_surface(mSurface);
    selector.set_minimum_version(1, 3);
    selector.set_required_features(features);
    selector.set_required_features_12(features_12);
    selector.set_required_features_13(features_13);

    auto physicalDevice = selector.select();

    if (!physicalDevice)
    {
        throw std::runtime_error("Failed to select physical device. Error: " + physicalDevice.error().message() + "\n");
    }

    mVkbPhysicalDevice = physicalDevice.value();
    mPhysicalDevice = mVkbPhysicalDevice.physical_device;
}

void Engine::CreateDevice()
{
    vkb::DeviceBuilder builder{mVkbPhysicalDevice};

    auto device = builder.build();

    if (!device)
    {
        throw std::runtime_error("Failed to create device. Error: " + device.error().message() + "\n");
    }

    mVkbDevice = device.value();
    mDevice = mVkbDevice.device;
}

void Engine::GetQueues()
{
    auto graphicsQueue = mVkbDevice.get_queue(vkb::QueueType::graphics);

    if (!graphicsQueue)
    {
        throw std::runtime_error("Failed to get graphics queue. Error: " + graphicsQueue.error().message() + "\n");
    }

    mGraphicsQueue = graphicsQueue.value();

    auto presentQueue = mVkbDevice.get_queue(vkb::QueueType::present);

    if (!presentQueue)
    {
        throw std::runtime_error("Failed to get present queue. Error: " + presentQueue.error().message() + "\n");
    }

    mPresentQueue = presentQueue.value();
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
    vkb::SwapchainBuilder builder{mVkbDevice};
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

void Engine::InitializePipelines()
{
    InitializeBackgroundPipeline();
    InitializeMeshPipeline();
    InitializeShadowmapPipeline();
}

void Engine::CreateCommandObjects()
{
    vkb::Result<u32> graphicsQueueIndex = mVkbDevice.get_queue_index(vkb::QueueType::graphics);

    if (!graphicsQueueIndex)
    {
        throw std::runtime_error("Failed to get graphics queue family index. Error: " + graphicsQueueIndex.error().message() + "\n");
    }

    VkCommandPoolCreateInfo poolInfo = CommandPoolCreateInfo(graphicsQueueIndex.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Per frame
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mFrames[i].commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool.");
        }

        VkCommandBufferAllocateInfo allocInfo = CommandBufferAllocateInfo(mFrames[i].commandPool, 1);

        if (vkAllocateCommandBuffers(mDevice, &allocInfo, &mFrames[i].mainCommandBuffer) != VK_SUCCESS)
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

void Engine::InitFrameDescriptorAllocators()
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

        mFrames[i].descriptorAllocator = DescriptorAllocator{};
        mFrames[i].descriptorAllocator.InitializePool(mDevice, 1000, sizes);
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
    //builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    mMaterialDescriptorLayout = builder.Build(mDevice, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Engine::InitDescriptors()
{
    InitGlobalDescriptorAllocator();
    InitFrameDescriptorAllocators();

    InitBackgroundDescriptorLayout();
    InitSceneDescriptorLayout();
    InitMaterialDescriptorLayout();

    mBackground.descriptorSet = mGlobalDescriptorAllocator.Allocate(mDevice, mBackground.descriptorLayout);

    DescriptorWriter writer;
    writer.WriteImage(0, mColorTarget.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(mDevice, mBackground.descriptorSet);
}

void Engine::InitBuffers()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
         mFrames[i].sceneDataBuffer = CreateBuffer(mAllocator, sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
}

void Engine::InitSyncObjects()
{
    VkFenceCreateInfo fenceInfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreInfo = SemaphoreCreateInfo();

    // Per frame
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mFrames[i].swapchainSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mFrames[i].renderSemaphore) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mFrames[i].renderFence) != VK_SUCCESS)
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

void Engine::Render()
{
    vkWaitForFences(mDevice, 1, &GetCurrentFrame().renderFence, VK_TRUE, UINT64_MAX);
    GetCurrentFrame().deletionQueue.Flush();
    GetCurrentFrame().descriptorAllocator.ClearDescriptors(mDevice);

    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mVkbSwapchain, UINT64_MAX, GetCurrentFrame().swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
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

    vkResetFences(mDevice, 1, &GetCurrentFrame().renderFence);
    vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0);

    VkCommandBuffer cmd = GetCurrentFrame().mainCommandBuffer;

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

    RenderGeometry(cmd);

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

    VkSemaphoreSubmitInfo waitInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetCurrentFrame().swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().renderSemaphore);

    VkSubmitInfo2 submitInfo = SubmitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    if (vkQueueSubmit2(mGraphicsQueue, 1, &submitInfo, GetCurrentFrame().renderFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
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

    mCurrentFrame++;
}

void Engine::RenderBackground(VkCommandBuffer pCmd)
{
    ComputeEffect& effect = mBackground.effects[mBackground.currentEffect];

    vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
    vkCmdBindDescriptorSets(pCmd, VK_PIPELINE_BIND_POINT_COMPUTE, mBackground.pipelineLayout, 0, 1, &mBackground.descriptorSet, 0, nullptr);

    vkCmdPushConstants(pCmd, mBackground.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);

    vkCmdDispatch(pCmd, std::ceil(mRenderExtent.width / 16.0), std::ceil(mRenderExtent.height / 16.0), 1);
}

void Engine::RenderImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(pTargetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = RenderingInfo(mVkbSwapchain.extent, &colorAttachment, nullptr);

    vkCmdBeginRendering(pCmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pCmd);

    vkCmdEndRendering(pCmd);
}

void Engine::RenderShadowmap(VkCommandBuffer pCmd)
{
    VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mShadowmapTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = RenderingInfo(VkExtent2D(mShadowmapTarget.extent.width, mShadowmapTarget.extent.height), VK_NULL_HANDLE, &depthAttachment);

    vkCmdBeginRendering(pCmd, &renderInfo);

    vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowmapPipeline);

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = static_cast<float>(mShadowmapTarget.extent.height);
    viewport.width = static_cast<float>(mShadowmapTarget.extent.width);
    viewport.height = -static_cast<float>(mShadowmapTarget.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(pCmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = mShadowmapTarget.extent.width;
    scissor.extent.height = mShadowmapTarget.extent.height;

    vkCmdSetScissor(pCmd, 0, 1, &scissor);

    for (auto& model : mApplication->mRenderContext.modelData)
    {
        ShadowmapPushConstants pushConstants;

        glm::vec3 lightPos = mApplication->mRenderContext.sceneData.mainLightPos;
        glm::mat4 matrixP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 100.0f, 0.1f);
        glm::mat4 matrixV = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

        pushConstants.depthMVP = matrixP * matrixV * model.transform;
        pushConstants.vertexBuffer = model.vertexBufferAddress;

        vkCmdPushConstants(pCmd, mShadowmapPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowmapPushConstants), &pushConstants);
        vkCmdBindIndexBuffer(pCmd, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(pCmd, model.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRendering(pCmd);
}

void Engine::RenderGeometry(VkCommandBuffer pCmd)
{
    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(mColorTarget.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mDepthTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = RenderingInfo(mRenderExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(pCmd, &renderInfo);

    vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline);

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = static_cast<float>(mRenderExtent.height);
    viewport.width = static_cast<float>(mRenderExtent.width);
    viewport.height = -static_cast<float>(mRenderExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(pCmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = mRenderExtent.width;
    scissor.extent.height = mRenderExtent.height;

    vkCmdSetScissor(pCmd, 0, 1, &scissor);

    float aspect = static_cast<float>(mRenderExtent.width) / static_cast<float>(mRenderExtent.height);

    SceneRenderData sceneRenderData = mApplication->mRenderContext.sceneData;
    glm::mat4 mainLightP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 100.0f, 0.1f);
    glm::mat4 mainLightV = glm::lookAt(sceneRenderData.mainLightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

    mScene.matrixV = glm::lookAt(sceneRenderData.cameraPos, sceneRenderData.cameraLookAt, glm::vec3(0.0f, 1.0f, 0.0f));
    mScene.matrixP = glm::perspective(glm::radians(sceneRenderData.cameraFOV), aspect, 100.0f, 0.1f);
    mScene.matrixVP = mScene.matrixP * mScene.matrixV;
    mScene.mainLightVP = mainLightP * mainLightV;
    mScene.mainLightColor = sceneRenderData.mainLightColor;
    mScene.mainLightDir = glm::normalize(glm::vec4(sceneRenderData.mainLightPos - glm::vec3(0.0f), 1.0f));
    mScene.ambientColor = glm::vec4(sceneRenderData.ambientColor, 1.0f);
    mScene.lightBuffer = mApplication->mRenderContext.lightData.lightBuffer;
    mScene.lightCount =  mApplication->mRenderContext.lightData.lightCount;
    mScene.cameraPos = glm::vec4(sceneRenderData.cameraPos, 0.0f);

    SceneData* sceneUniformData = static_cast<SceneData*>(GetCurrentFrame().sceneDataBuffer.info.pMappedData);
    *sceneUniformData = mScene;

    VkDescriptorSet sceneSet = GetCurrentFrame().descriptorAllocator.Allocate(mDevice, mSceneDescriptorLayout);

    DescriptorWriter writer;
    writer.WriteBuffer(0, GetCurrentFrame().sceneDataBuffer.buffer, sizeof(SceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.WriteImage(1, mShadowmapTarget.imageView, TextureManager::Get().GetSampler("NEAREST_MIPMAP_LINEAR"), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.UpdateSet(mDevice, sceneSet);

    vkCmdBindDescriptorSets(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 0, 1, &sceneSet, 0, nullptr);

    for (auto& model : mApplication->mRenderContext.modelData)
    {
        GeometryPushConstants pushConstants;
        pushConstants.matrixM = model.transform;
        pushConstants.matrixITM = glm::transpose(glm::inverse(model.transform));
        pushConstants.vertexBuffer = model.vertexBufferAddress;

        vkCmdPushConstants(pCmd, mMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GeometryPushConstants), &pushConstants);
        vkCmdBindDescriptorSets(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipelineLayout, 1, 1, &model.materialSet, 0, nullptr);
        vkCmdBindIndexBuffer(pCmd, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(pCmd, model.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRendering(pCmd);
}

void Engine::InitializeBackgroundPipeline()
{
    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &mBackground.descriptorLayout;
    computeLayout.setLayoutCount = 1;
    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(mDevice, &computeLayout, nullptr, &mBackground.pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    VkShaderModule gradientShader;
    LoadShaderModule("assets/shaders/gradient_color.spv", mDevice, &gradientShader);

    VkPipelineShaderStageCreateInfo stageInfo = PipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, gradientShader, "main");

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = mBackground.pipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    ComputeEffect gradient{};
    gradient.layout = mBackground.pipelineLayout;
    gradient.name = "gradient";
    gradient.data.data1 = glm::vec4(0, 0, 0, 1);
    gradient.data.data2 = glm::vec4(0, 0, 0, 1);

    if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline.");
    }

    mBackground.effects.push_back(gradient);

    vkDestroyShaderModule(mDevice, gradientShader, nullptr);
}

void Engine::InitializeMeshPipeline()
{
    VkShaderModule vertexShader;
    LoadShaderModule("assets/shaders/lit_phong_vert.spv", mDevice, &vertexShader);

    VkShaderModule fragmentShader;
    LoadShaderModule("assets/shaders/lit_phong_frag.spv", mDevice, &fragmentShader);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GeometryPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayout, 2> descriptorSets = { mSceneDescriptorLayout, mMaterialDescriptorLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSets.data();
    pipelineLayoutInfo.setLayoutCount = 2;
    VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mMeshPipelineLayout));

    PipelineBuilder builder;
    builder.mPipelineLayout = mMeshPipelineLayout;
    builder.SetShaders(vertexShader, fragmentShader);
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    builder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    builder.SetColorAttachmentFormat(mColorTarget.format);
    builder.SetDepthFormat(mDepthTarget.format);
    mMeshPipeline = builder.Build(mDevice);

    vkDestroyShaderModule(mDevice, vertexShader, nullptr);
    vkDestroyShaderModule(mDevice, fragmentShader, nullptr);
}

void Engine::InitializeShadowmapPipeline()
{
    VkShaderModule vertexShader;
    LoadShaderModule("assets/shaders/shadowmap_vert.spv", mDevice, &vertexShader);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ShadowmapPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mShadowmapPipelineLayout));

    PipelineBuilder builder;
    builder.mPipelineLayout = mShadowmapPipelineLayout;
    builder.SetShaders(vertexShader, VK_NULL_HANDLE);
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    builder.SetCullMode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    builder.SetDepthFormat(mShadowmapTarget.format);
    mShadowmapPipeline = builder.Build(mDevice);

    vkDestroyShaderModule(mDevice, vertexShader, nullptr);
}

MeshBuffers Engine::UploadMesh(std::span<u32> pIndices, std::span<Vertex> pVertices)
{
    const size_t vertexBufferSize = pVertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = pIndices.size() * sizeof(u32);

    MeshBuffers newSurface;

    VkBufferUsageFlags vertexBufferUsage{};
    vertexBufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    vertexBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vertexBufferUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    newSurface.vertexBuffer = CreateBuffer(mAllocator, vertexBufferSize, vertexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = newSurface.vertexBuffer.buffer;
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(mDevice, &deviceAddressInfo);

    VkBufferUsageFlags indexBufferUsage{};
    indexBufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    newSurface.indexBuffer = CreateBuffer(mAllocator, indexBufferSize, indexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY);

    VulkanBuffer stagingBuffer = CreateBuffer(mAllocator, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = stagingBuffer.info.pMappedData;

    memcpy(data, pVertices.data(), vertexBufferSize);
    memcpy(static_cast<char *>(data) + vertexBufferSize, pIndices.data(), indexBufferSize);

    auto func = [&](VkCommandBuffer pCmd)
    {
        VkBufferCopy vertexCopy{};
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(pCmd, stagingBuffer.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(pCmd, stagingBuffer.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
    };

    ImmediateSubmit(mDevice, mGraphicsQueue, mImmediate, func);

    DestroyBuffer(mAllocator, stagingBuffer);

    return newSurface;
}

void Engine::copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Engine::BeginSingleTimeCommands()
{
    VkCommandBuffer cmd;

    VkCommandBufferAllocateInfo allocInfo = CommandBufferAllocateInfo(GetCurrentFrame().commandPool, 1);
    vkAllocateCommandBuffers(mDevice, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(cmd, &beginInfo);

    return cmd;
}

void Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);

    vkFreeCommandBuffers(mDevice, GetCurrentFrame().commandPool, 1, &commandBuffer);
}
