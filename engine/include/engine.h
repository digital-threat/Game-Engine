#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <application.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vendor/stb/stb_image.h>
#include <VkBootstrap.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vector>
#include <array>
#include <functional>
#include <cassert>
#include <entity_manager.h>
#include <iostream>
#include <mesh_manager.h>
#include <renderer_vk_images.h>
#include <texture_manager.h>

#include "types.h"
#include "renderer_vk_types.h"
#include "renderer_vk_descriptors.h"
#include "renderer_vk_buffers.h"
#include "utility.h"

using namespace Renderer;

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

    VkSampler mSamplerLinear;
    VkSampler mSamplerNearest;

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

    VkDescriptorSetLayout mSingleImageDescriptorLayout;

    VkExtent2D mRenderExtent{};

    // Pipelines
    VkPipelineLayout mMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mMeshPipeline = VK_NULL_HANDLE;

    // Immediate
    ImmediateData mImmediate{};

    Application* mApplication;

public:
    Engine(Application* application) : mApplication(application)
    {
        MeshManager::Allocate(*this);
        TextureManager::Allocate(*this);
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
        InitTextureSamplers();
        InitializePipelines();
    }

    void MainLoop()
    {
        mApplication->Awake();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            if (mResizeRequested)
            {
                ResizeSwapchain();
            }

            mApplication->Update();
            MeshManager &meshManger = MeshManager::Get();
            meshManger.ProcessMessages();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            mApplication->Render();
            ImGui::Render();
            Draw();
        }

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
    void InitTextureSamplers();

    // Descriptors
    void InitDescriptors();
    void InitGlobalDescriptorAllocator();
    void InitFrameDescriptorAllocators();
    void InitBackgroundDescriptorLayout();
    void InitSceneDescriptorLayout();
    void InitSingleImageDescriptorLayout();

    // Buffers
    void InitBuffers();

    // Drawing
    void Draw();
    void DrawBackground(VkCommandBuffer pCmd);
    void DrawImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView);
    void DrawGeometry(VkCommandBuffer pCmd);

    // Pipelines
    void InitializePipelines();
    void InitializeBackgroundPipeline();
    void InitializeMeshPipeline();

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
