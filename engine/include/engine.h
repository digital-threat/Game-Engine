#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS

#include <VkBootstrap.h>

#include <vector>

#include <application.h>
#include <material_manager.h>
#include <mesh_manager.h>
#include <vk_images.h>
#include <texture_manager.h>
#include <vk_immediate.h>
#include <types.h>
#include <vk_types.h>
#include <vk_descriptors.h>


constexpr u32 WIDTH = 1280;
constexpr u32 HEIGHT = 720;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct ComputeEffect
{
    const char* name;

    VkPipeline pipeline;
    VkPipelineLayout layout;

    ComputePushConstants data;
};

class Engine
{
public:
    // Resize window
    bool mResizeRequested = false;
    bool mFramebufferResized = false;

    GLFWwindow* mWindow = nullptr;

    VmaAllocator mAllocator{};

    vkb::Swapchain mVkbSwapchain;
    vkb::DispatchTable mVkbDispatchTable;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    // Immediate
    ImmediateData mImmediate{};

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    u32 mGraphicsQueueIndex;
    u32 mPresentQueueIndex;

    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;

    VulkanImage mColorTarget{};
    VulkanImage mDepthTarget{};
    VulkanImage mShadowmapTarget{};


    DescriptorAllocator mGlobalDescriptorAllocator{};

    struct BackgroundData
    {
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        std::vector<ComputeEffect> effects;
        int currentEffect = 0;
    };

    BackgroundData mBackground{};

    VkDescriptorSetLayout mSceneDescriptorLayout;
    VkDescriptorSetLayout mMaterialDescriptorLayout;

    VkExtent2D mRenderExtent{};

    // Pipelines
    VkPipelineLayout mMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mMeshPipeline = VK_NULL_HANDLE;

    VkPipelineLayout mShadowmapPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mShadowmapPipeline = VK_NULL_HANDLE;

    Application* mApplication;

    // Ray tracing
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR mRtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
    VkDescriptorSetLayout mRaytracingDescriptorLayout;

public:
    Engine()
    {
        MeshManager::Allocate(*this);
        TextureManager::Allocate(*this);
        MaterialManager::Allocate(*this);
    }

    void Run(Application* application);

private:
    void InitWindow();
    void InitVulkan(FrameData* frames);
    void MainLoop(FrameData* frames);
    void Cleanup();

    void ResizeSwapchain();

    static vkb::Instance CreateInstance();
    void CreateSurface();
    static vkb::PhysicalDevice SelectPhysicalDevice(vkb::Instance& vkbInstance, VkSurfaceKHR vkSurface);
    vkb::Device CreateDevice(vkb::PhysicalDevice& vkbPhysicalDevice);
    void CreateDispatchTable(vkb::Device vkbDevice);
    void GetQueues(vkb::Device& device);
    void CreateAllocator();
    void CreateSwapchain(u32 width, u32 height);
    void CreateCommandObjects(vkb::Device& device, FrameData* frames);
    void InitSyncObjects(FrameData* frames);

    // Descriptors
    void InitDescriptors(FrameData* frames);
    void InitGlobalDescriptorAllocator();
    void InitFrameDescriptorAllocators(FrameData* frames);

    // Buffers
    void InitBuffers(FrameData* frames);

    // Drawing
    void Render(FrameData& currentFrame);

    // Pipelines
    void InitPipelines();

    // ImGui
    void InitImGui();
    void RenderImgui(VkCommandBuffer cmd, VkImageView targetImageView);

    // Rasterized
    void InitRasterSceneDescriptorLayout();
    void InitRasterMaterialDescriptorLayout();
    void InitRasterPipeline();
    void RenderRaster(VkCommandBuffer cmd, FrameData& currentFrame);

    // Rasterized shadow mapping
    void InitShadowmapPipeline();
    void RenderShadowmap(VkCommandBuffer cmd);

    // Ray tracing
    void InitRaytracing();
    void InitRaytracingDescriptorLayout();
    void InitRaytracingSceneDescriptorLayout();
    void InitRaytracingMaterialDescriptorLayout();
    void InitRaytracingPipeline();
    void RenderRaytracing(VkCommandBuffer cmd, FrameData& currentFrame);

    // Compute background
    void InitBackgroundDescriptorLayout();
    void UpdateBackgroundDescriptorSet();
    void InitBackgroundPipeline();
    void RenderBackground(VkCommandBuffer cmd);

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->mFramebufferResized = true;
    }

public:
    void UploadMesh(CpuMesh& inMesh, GpuMesh& outMesh);
};
