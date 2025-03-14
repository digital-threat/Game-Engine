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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vector>
#include <functional>
#include <thread>

#include <application.h>
#include <input.h>
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

    GLFWwindow *mWindow = nullptr;

    VmaAllocator mAllocator{};

    vkb::Instance mVkbInstance;
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

public:
    Engine(Application* application) : mApplication(application)
    {
        MeshManager::Allocate(*this);
        TextureManager::Allocate(*this);
        MaterialManager::Allocate(*this);
    }

    void Run()
    {
        FrameData frames[MAX_FRAMES_IN_FLIGHT]{};

        InitWindow();
        Input::Initialize(mWindow);
        InitVulkan(frames);
        InitImGui();
        MainLoop(frames);
        Cleanup();
    }

private:
    void InitWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
    }

    void InitVulkan(FrameData* frames)
    {
        CreateInstance();
        CreateSurface();
        auto vkbPhysicalDevice = SelectPhysicalDevice();
        auto vkbDevice = CreateDevice(vkbPhysicalDevice);
        GetQueues(vkbDevice);
        CreateAllocator();
        CreateSwapchain(WIDTH, HEIGHT);
        InitSyncObjects(frames);
        CreateCommandObjects(vkbDevice, frames);
        InitDescriptors(frames);
        InitBuffers(frames);
        InitializePipelines();
    }

    void MainLoop(FrameData* frames)
    {
        u32 currentFrame = 0;

        MeshManager &meshManger = MeshManager::Get();
        std::atomic<bool> cancellationToken;
        std::thread meshManagerThread(&MeshManager::Update, &meshManger, std::ref(cancellationToken));

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
    vkb::PhysicalDevice SelectPhysicalDevice();
    vkb::Device CreateDevice(vkb::PhysicalDevice& vkbPhysicalDevice);
    void GetQueues(vkb::Device& device);
    void CreateAllocator();
    void CreateSwapchain(u32 width, u32 height);
    void CreateCommandObjects(vkb::Device& device, FrameData* frames);
    void InitSyncObjects(FrameData* frames);

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
    void RenderBackground(VkCommandBuffer pCmd);
    void RenderImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView);
    void RenderShadowmap(VkCommandBuffer pCmd);
    void RenderGeometry(VkCommandBuffer pCmd, FrameData& currentFrame);

    // Pipelines
    void InitializePipelines();
    void InitializeBackgroundPipeline();
    void InitializeMeshPipeline();
    void InitializeShadowmapPipeline();

    // ImGui
    void InitImGui();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->mFramebufferResized = true;
    }

public:
    MeshBuffers UploadMesh(std::span<u32> pIndices, std::span<Vertex> pVertices);
};
