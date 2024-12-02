#include "engine.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstring>
#include <vk_mem_alloc.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "renderer_vk_images.h"
#include "renderer_vk_pipelines.h"
#include "renderer_vk_structures.h"

void Engine::InitImgui()
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

void Engine::LoadMeshes()
{
    std::vector<MeshAsset*> box = LoadGltfMeshes(this, "assets/meshes/Box.glb");
    mEntities.reserve(3);

    for (int i = 0; i < 3; i++)
    {
        mEntities.emplace_back();
        mEntities[i].name = "Default Name";
        mEntities[i].mesh = box[0];
        mEntities[i].position = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        mEntities[i].rotation = glm::vec3();
        mEntities[i].scale = 1;
    }
}

void Engine::RecreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    CleanupSwapchain();

    CreateSwapchain();
    createDepthResources();
}

void Engine::CleanupSwapchain()
{
    vkDestroyImageView(mDevice, depthImageView, nullptr);
    vkDestroyImage(mDevice, depthImage, nullptr);
    vkFreeMemory(mDevice, depthImageMemory, nullptr);

    for (auto &framebuffer : mSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }

    mVkbSwapchain.destroy_image_views(mSwapchainImageViews);
}

void Engine::CreateInstance()
{
    vkb::InstanceBuilder builder;
    builder.set_app_name ("Application");
    builder.set_engine_name("No Engine");
    builder.request_validation_layers(enableValidationLayers);
    builder.use_default_debug_messenger();
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

void Engine::CreateSwapchain()
{
    vkb::SwapchainBuilder builder{mVkbDevice};
    builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    builder.set_old_swapchain(mVkbSwapchain);
    builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    auto swapchain = builder.build();

    if (!swapchain)
    {
        std::cout << swapchain.vk_result() << std::endl;
        throw std::runtime_error("Failed to create swapchain. Error: " + swapchain.error().message() + "\n");
    }

    vkb::destroy_swapchain(mVkbSwapchain);
    mVkbSwapchain = swapchain.value();

    mSwapchainImages = mVkbSwapchain.get_images().value();
    mSwapchainImageViews = mVkbSwapchain.get_image_views().value();

    mColorTarget.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    mColorTarget.extent = {mVkbSwapchain.extent.width, mVkbSwapchain.extent.height, 1};

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

    mDepthTarget.format = VK_FORMAT_D32_SFLOAT;
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
}

void Engine::InitializePipelines()
{
    InitializeBackgroundPipeline();
    InitializeMeshPipeline();
}

void Engine::CreateCommandObjects()
{
    auto graphicsQueueIndex = mVkbDevice.get_queue_index(vkb::QueueType::graphics);

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
    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mImmediateCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }

    VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(mImmediateCommandPool, 1);
    if (vkAllocateCommandBuffers(mDevice, &cmdAllocInfo, &mImmediateCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers.");
    }
}

void Engine::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createImage(mVkbSwapchain.extent.width, mVkbSwapchain.extent.height, depthFormat, tiling, usage, properties, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Engine::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("assets/textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    constexpr VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    constexpr VkMemoryPropertyFlags bufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    createBuffer(imageSize, bufferUsage, bufferProperties, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    constexpr VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    constexpr VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
    constexpr VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    constexpr VkMemoryPropertyFlags imageProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createImage(texWidth, texHeight, imageFormat, imageTiling, imageUsage, imageProperties, textureImage, textureImageMemory);

    VkCommandBuffer cmd = BeginSingleTimeCommands();
    TransitionImageLayout(cmd, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<u32>(texWidth), static_cast<u32>(texHeight));
    TransitionImageLayout(cmd, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    endSingleTimeCommands(cmd);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Engine::createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Engine::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Engine::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        constexpr VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        createBuffer(bufferSize, usage, properties, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(mDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void Engine::InitializeDescriptors()
{
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
    };

    mGlobalDescriptorAllocator.InitializePool(mDevice, 10, sizes);

    {
        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        mRenderTargetDescriptorSetLayout = builder.Build(mDevice, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    mRenderTargetDescriptorSet = mGlobalDescriptorAllocator.Allocate(mDevice, mRenderTargetDescriptorSetLayout);

    VkDescriptorImageInfo info{};
    info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    info.imageView = mColorTarget.imageView;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstSet = mRenderTargetDescriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = &info;

    vkUpdateDescriptorSets(mDevice, 1, &writeDescriptorSet, 0, nullptr);


    // std::array<VkDescriptorPoolSize, 2> poolSizes{};
    // poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // poolSizes[0].descriptorCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT);
    // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // poolSizes[1].descriptorCount = static_cast<u32>(MAX_FRAMES_IN_FLIGHT);

    // VkDescriptorSetLayoutBinding uboLayoutBinding{};
    // uboLayoutBinding.binding = 0;
    // uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // uboLayoutBinding.descriptorCount = 1;
    // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //
    // VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    // samplerLayoutBinding.binding = 1;
    // samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // samplerLayoutBinding.descriptorCount = 1;
    // samplerLayoutBinding.pImmutableSamplers = nullptr;
    // samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // VkDescriptorBufferInfo bufferInfo{};
    // bufferInfo.buffer = uniformBuffers[i];
    // bufferInfo.offset = 0;
    // bufferInfo.range = sizeof(UniformBufferObject);
    //
    // VkDescriptorImageInfo imageInfo{};
    // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // imageInfo.imageView = textureImageView;
    // imageInfo.sampler = textureSampler;
    //
    // std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    //
    // descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrites[0].dstSet = descriptorSets[i];
    // descriptorWrites[0].dstBinding = 0;
    // descriptorWrites[0].dstArrayElement = 0;
    // descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // descriptorWrites[0].descriptorCount = 1;
    // descriptorWrites[0].pBufferInfo = &bufferInfo;
    //
    // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrites[1].dstSet = descriptorSets[i];
    // descriptorWrites[1].dstBinding = 1;
    // descriptorWrites[1].dstArrayElement = 0;
    // descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // descriptorWrites[1].descriptorCount = 1;
    // descriptorWrites[1].pImageInfo = &imageInfo;
    //
    // vkUpdateDescriptorSets(mDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void Engine::CreateSyncObjects()
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
    if(vkCreateFence(mDevice, &fenceInfo, nullptr, &mImmediateFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence.");
    }
}

void Engine::Draw()
{
    vkWaitForFences(mDevice, 1, &GetCurrentFrame().renderFence, VK_TRUE, UINT64_MAX);
    GetCurrentFrame().deletionQueue.Flush();

    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mVkbSwapchain, UINT64_MAX, GetCurrentFrame().swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image.");
    }

    //updateUniformBuffer(mCurrentFrame);

    vkResetFences(mDevice, 1, &GetCurrentFrame().renderFence);
    vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0);

    VkCommandBuffer cmd = GetCurrentFrame().mainCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    DrawBackground(cmd, imageIndex);

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionImageLayout(cmd, mDepthTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    DrawGeometry(cmd);

    TransitionImageLayout(cmd, mColorTarget.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout(cmd, mSwapchainImages[imageIndex],VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkExtent2D drawExtent = {mColorTarget.extent.width, mColorTarget.extent.height};
    CopyImage(cmd, mColorTarget.image, mSwapchainImages[imageIndex], drawExtent, mVkbSwapchain.extent);

    TransitionImageLayout(cmd, mSwapchainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    DrawImgui(cmd, mSwapchainImageViews[imageIndex]);


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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    mCurrentFrame++;
}

void Engine::DrawBackground(const VkCommandBuffer pCmd, const u32 pImageIndex)
{
    ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];

    vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
    vkCmdBindDescriptorSets(pCmd, VK_PIPELINE_BIND_POINT_COMPUTE, mBackgroundPipelineLayout, 0, 1, &mRenderTargetDescriptorSet, 0, nullptr);

    vkCmdPushConstants(pCmd, mBackgroundPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);

    vkCmdDispatch(pCmd, std::ceil(mColorTarget.extent.width / 16.0), std::ceil(mColorTarget.extent.height / 16.0), 1);

    // VkCommandBufferBeginInfo beginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    //
    // if (vkBeginCommandBuffer(pCmd, &beginInfo) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("Failed to begin recording command buffer.");
    // }
    //
    // TransitionImageLayout(pCmd, mSwapchainImages[pImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    //
    // VkRenderPassBeginInfo renderPassInfo{};
    // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // renderPassInfo.renderPass = renderPass;
    // renderPassInfo.framebuffer = mSwapchainFramebuffers[pImageIndex];
    // renderPassInfo.renderArea.offset = {0, 0};
    // renderPassInfo.renderArea.extent = mVkbSwapchain.extent;
    //
    // std::array<VkClearValue, 2> clearValues{};
    // clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    // clearValues[1].depthStencil = {1.0f, 0};
    //
    // renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
    // renderPassInfo.pClearValues = clearValues.data();
    //
    // vkCmdBeginRenderPass(pCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    //
    // VkViewport viewport{};
    // viewport.x = 0.0f;
    // viewport.y = static_cast<float>(mVkbSwapchain.extent.height);
    // viewport.width = static_cast<float>(mVkbSwapchain.extent.width);
    // viewport.height = -static_cast<float>(mVkbSwapchain.extent.height);
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;
    // vkCmdSetViewport(pCmd, 0, 1, &viewport);
    //
    // VkRect2D scissor{};
    // scissor.offset = {0, 0};
    // scissor.extent = mVkbSwapchain.extent;
    // vkCmdSetScissor(pCmd, 0, 1, &scissor);
    //
    // VkBuffer vertexBuffers[] = {vertexBuffer};
    // VkDeviceSize offsets[] = {0};
    // vkCmdBindVertexBuffers(pCmd, 0, 1, vertexBuffers, offsets);
    //
    // // NOTE(Sergei): Nasty hardcoded index type!
    // vkCmdBindIndexBuffer(pCmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    //
    // vkCmdBindDescriptorSets(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[mCurrentFrame], 0, nullptr);
    // vkCmdDrawIndexed(pCmd, static_cast<u32>(indices.size()), 1, 0, 0, 0);
    // vkCmdEndRenderPass(pCmd);
    //
    // if (vkEndCommandBuffer(pCmd) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to record command buffer!");
    // }
}

void Engine::DrawImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(pTargetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = RenderingInfo(mVkbSwapchain.extent, &colorAttachment, nullptr);

    vkCmdBeginRendering(pCmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pCmd);

    vkCmdEndRendering(pCmd);
}

void Engine::DrawGeometry(VkCommandBuffer pCmd)
{
    VkRenderingAttachmentInfo colorAttachment = ColorAttachmentInfo(mColorTarget.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = DepthAttachmentInfo(mDepthTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkExtent2D drawExtent = { mColorTarget.extent.width, mColorTarget.extent.height };
    VkRenderingInfo renderInfo = RenderingInfo(drawExtent, &colorAttachment, &depthAttachment);

    vkCmdBeginRendering(pCmd, &renderInfo);

    vkCmdBindPipeline(pCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline);

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = static_cast<float>(drawExtent.height);
    viewport.width = static_cast<float>(drawExtent.width);
    viewport.height = -static_cast<float>(drawExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(pCmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = drawExtent.width;
    scissor.extent.height = drawExtent.height;

    vkCmdSetScissor(pCmd, 0, 1, &scissor);


    GeometryPushConstants pushConstants;

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();

    float aspect = static_cast<float>(mColorTarget.extent.width) / static_cast<float>(mColorTarget.extent.height);
    glm::mat4 view = glm::lookAt(mCamera.position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(mCamera.fov), aspect, 10000.0f, 0.1f);

    for (int i = 0; i < mEntities.size(); i++)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), mEntities[i].position);
        glm::quat rotation = glm::quat(radians(mEntities[i].rotation));
        model *= glm::toMat4(rotation);
        model = glm::scale(model, glm::vec3(mEntities[i].scale));


        pushConstants.worldMatrix = projection * view * model;
        pushConstants.vertexBuffer = mEntities[i].mesh->meshBuffers.vertexBufferAddress;

        vkCmdPushConstants(pCmd, mMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GeometryPushConstants), &pushConstants);
        vkCmdBindIndexBuffer(pCmd, mEntities[i].mesh->meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(pCmd, mEntities[i].mesh->submeshes[0].count, 1, mEntities[i].mesh->submeshes[0].startIndex, 0, 0);
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
    computeLayout.pSetLayouts = &mRenderTargetDescriptorSetLayout;
    computeLayout.setLayoutCount = 1;
    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(mDevice, &computeLayout, nullptr, &mBackgroundPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    VkShaderModule gradientShader;
    LoadShaderModule("assets/shaders/gradient_color.spv", mDevice, &gradientShader);

    VkShaderModule skyShader;
    LoadShaderModule("assets/shaders/sky.spv", mDevice, &skyShader);

    VkPipelineShaderStageCreateInfo stageInfo = PipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, gradientShader, "main");

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = mBackgroundPipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    ComputeEffect gradient{};
    gradient.layout = mBackgroundPipelineLayout;
    gradient.name = "gradient";
    gradient.data.data1 = glm::vec4(1, 0, 0, 1);
    gradient.data.data2 = glm::vec4(0, 0, 1, 1);

    if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline.");
    }

    computePipelineCreateInfo.stage.module = skyShader;

    ComputeEffect sky{};
    sky.layout = mBackgroundPipelineLayout;
    sky.name = "sky";
    sky.data.data1 = glm::vec4(0.1, 0.2, 0.4 ,0.97);

    if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline.");
    }

    backgroundEffects.push_back(gradient);
    backgroundEffects.push_back(sky);

    vkDestroyShaderModule(mDevice, gradientShader, nullptr);
    vkDestroyShaderModule(mDevice, skyShader, nullptr);

}

void Engine::InitializeMeshPipeline()
{
    VkShaderModule vertexShader;
    LoadShaderModule("assets/shaders/colored_triangle_vert.spv", mDevice, &vertexShader);

    VkShaderModule fragmentShader;
    LoadShaderModule("assets/shaders/colored_triangle_frag.spv", mDevice, &fragmentShader);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GeometryPushConstants);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = PipelineLayoutCreateInfo();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    VK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mMeshPipelineLayout));

    PipelineBuilder builder;
    builder.mPipelineLayout = mMeshPipelineLayout;
    builder.SetShaders(vertexShader, fragmentShader);
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    builder.SetColorAttachmentFormat(mColorTarget.format);
    builder.SetDepthFormat(mDepthTarget.format);
    mMeshPipeline = builder.Build(mDevice);

    vkDestroyShaderModule(mDevice, vertexShader, nullptr);
    vkDestroyShaderModule(mDevice, fragmentShader, nullptr);
}

void Engine::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&pFunction)
{
    VK_CHECK(vkResetFences(mDevice, 1, &mImmediateFence));
    VK_CHECK(vkResetCommandBuffer(mImmediateCommandBuffer, 0));

    VkCommandBuffer cmd = mImmediateCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    pFunction(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = CommandBufferSubmitInfo(cmd);
    VkSubmitInfo2 submit = SubmitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(mGraphicsQueue, 1, &submit, mImmediateFence));

    VK_CHECK(vkWaitForFences(mDevice, 1, &mImmediateFence, true, UINT64_MAX));
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

    ImmediateSubmit(func);

    DestroyBuffer(mAllocator, stagingBuffer);

    return newSurface;
}

u32 Engine::findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}

void Engine::createImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(mDevice, image, imageMemory, 0);
}

void Engine::copyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Engine::updateUniformBuffer(u32 currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    float aspect = static_cast<float>(mVkbSwapchain.extent.width) / static_cast<float>(mVkbSwapchain.extent.height);

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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

VkImageView Engine::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    auto viewInfo = ImageViewCreateInfo(image, format, aspectFlags);

    VkImageView imageView;
    if (vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

VkFormat Engine::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat Engine::findDepthFormat()
{
    std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    return findSupportedFormat(candidates, tiling, features);
}

bool Engine::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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
