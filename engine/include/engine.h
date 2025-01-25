#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#include <VkBootstrap.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vector>
#include <functional>
#include <thread>

#include <application.h>
#include <material_manager.h>
#include <mesh_manager.h>
#include <renderer_vk_images.h>
#include <texture_manager.h>

#include "types.h"
#include "renderer_vk_types.h"
#include "renderer_vk_descriptors.h"
#include "utility.h"

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

    GLFWwindow *window = nullptr;

    VmaAllocator mAllocator{};

    vkb::Instance mVkbInstance;
    vkb::PhysicalDevice mVkbPhysicalDevice;
    vkb::Device mVkbDevice;
    vkb::Swapchain mVkbSwapchain;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    FrameData mFrames[MAX_FRAMES_IN_FLIGHT]{};
    u32 mCurrentFrame = 0;

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;

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

    // Immediate
    ImmediateData mImmediate{};

    Application* mApplication;

public:
    Engine(Application* application) : mApplication(application)
    {
        MeshManager::Allocate(*this);
        TextureManager::Allocate(*this);
        MaterialManager::Allocate(*this);
    }

    void Run()
    {
        InitWindow();
        InitVulkan();
        InitImGui();
        MainLoop();
        Cleanup();
    }

private:
    void InitWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
    }

    void InitVulkan()
    {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateDevice();
        GetQueues();
        CreateAllocator();
        CreateSwapchain(WIDTH, HEIGHT);
        InitSyncObjects();
        CreateCommandObjects();
        InitDescriptors();
        InitBuffers();
        InitializePipelines();
    }

    void MainLoop()
    {
        MeshManager &meshManger = MeshManager::Get();
        std::atomic<bool> cancellationToken;
        std::thread meshManagerThread(&MeshManager::Update, &meshManger, std::ref(cancellationToken));

        TextureManager::Get().Awake();
        mApplication->Awake();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            if (mResizeRequested)
            {
                ResizeSwapchain();
            }

            mApplication->Update();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            mApplication->Render();
            ImGui::Render();
            Render();
        }

        cancellationToken = true;
        meshManagerThread.join();

        vkDeviceWaitIdle(mDevice);

        mApplication->Destroy();
    }

    void Cleanup()
    {
        glfwTerminate();
    }

    void ResizeSwapchain();

    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateDevice();
    void GetQueues();
    void CreateAllocator();
    void CreateSwapchain(u32 width, u32 height);
    void CreateCommandObjects();
    void InitSyncObjects();

    // Descriptors
    void InitDescriptors();
    void InitGlobalDescriptorAllocator();
    void InitFrameDescriptorAllocators();
    void InitBackgroundDescriptorLayout();
    void InitSceneDescriptorLayout();
    void InitMaterialDescriptorLayout();

    // Buffers
    void InitBuffers();

    // Drawing
    void Render();
    void RenderBackground(VkCommandBuffer pCmd);
    void RenderImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView);
    void RenderShadowmap(VkCommandBuffer pCmd);
    void RenderGeometry(VkCommandBuffer pCmd);

    // Pipelines
    void InitializePipelines();
    void InitializeBackgroundPipeline();
    void InitializeMeshPipeline();
    void InitializeShadowmapPipeline();

    // ImGui
    void InitImGui();

    FrameData& GetCurrentFrame() { return mFrames[mCurrentFrame % MAX_FRAMES_IN_FLIGHT]; }

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->mFramebufferResized = true;
    }

public:
    MeshBuffers UploadMesh(std::span<u32> pIndices, std::span<Vertex> pVertices);

private:
    // Old
    void copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);
    VkCommandBuffer BeginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};
