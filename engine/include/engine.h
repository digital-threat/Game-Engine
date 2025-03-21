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
#include <vk_utility.h>
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

    GLFWwindow *mWindow = nullptr;

    VmaAllocator mAllocator{};

    vkb::Swapchain mVkbSwapchain;

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

    SceneData mScene{};
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

public:
    Engine(Application* application) : mApplication(application)
    {
        MeshManager::Allocate(*this);
        TextureManager::Allocate(*this);
        MaterialManager::Allocate(*this);
    }

    void Run();

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
    void GetQueues(vkb::Device& device);
    void CreateAllocator();
    void CreateSwapchain(u32 width, u32 height);
    void CreateCommandObjects(vkb::Device& device, FrameData* frames);
    void InitSyncObjects(FrameData* frames);

    // Ray tracing
    void InitRaytracing();

    // Descriptors
    void InitDescriptors(FrameData* frames);
    void InitGlobalDescriptorAllocator();
    void InitFrameDescriptorAllocators(FrameData* frames);
    void InitBackgroundDescriptorLayout();
    void InitSceneDescriptorLayout();
    void InitMaterialDescriptorLayout();

    // Buffers
    void InitBuffers(FrameData* frames);

    // Drawing
    void Render(FrameData& currentFrame);
    void RenderBackground(VkCommandBuffer cmd);
    void RenderImgui(VkCommandBuffer cmd, VkImageView targetImageView);
    void RenderRasterized(VkCommandBuffer cmd, FrameData& currentFrame);
    void RenderShadowmap(VkCommandBuffer cmd);
    void RenderRaytracing(VkCommandBuffer cmd);

    // Pipelines
    void InitPipelines();
    void InitBackgroundPipeline();
    void InitRasterizedPipeline();
    void InitShadowmapPipeline();
    void InitRaytracingPipeline();

    // ImGui
    void InitImGui();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->mFramebufferResized = true;
    }

public:
    void UploadMesh(std::span<u32> indices, std::span<Vertex> vertices, GpuMesh& mesh);
};
