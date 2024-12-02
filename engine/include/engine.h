#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
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
#include <iostream>

#include "types.h"
#include "renderer_vk_types.h"
#include "renderer_vk_descriptors.h"
#include "renderer_vk_buffers.h"
#include "gltf_loading.h"
#include "utility.h"
#include "entity.h"
#include "camera.h"

using namespace Renderer;

constexpr u32 WIDTH = 800;
constexpr u32 HEIGHT = 600;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct ComputeEffect
{
    const char* name;

    VkPipeline pipeline;
    VkPipelineLayout layout;

    ComputePushConstants data;
};

class Engine
{
private:
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
    std::vector<VkFramebuffer> mSwapchainFramebuffers;

    VulkanImage mColorTarget{};
    VulkanImage mDepthTarget{};

    DescriptorAllocator mGlobalDescriptorAllocator{};

    VkDescriptorSet mRenderTargetDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout mRenderTargetDescriptorSetLayout = VK_NULL_HANDLE;

    // Pipelines
    VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

    VkPipelineLayout mBackgroundPipelineLayout = VK_NULL_HANDLE;
    std::vector<ComputeEffect> backgroundEffects;
    int currentBackgroundEffect = 0;

    VkPipelineLayout mMeshPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mMeshPipeline = VK_NULL_HANDLE;

    // Immediate
    VkFence mImmediateFence = VK_NULL_HANDLE;
    VkCommandBuffer mImmediateCommandBuffer = VK_NULL_HANDLE;
    VkCommandPool mImmediateCommandPool = VK_NULL_HANDLE;

    // Other
    Camera mCamera{ .position = glm::vec3{ 0.0f, 2.0f, -3.0f }, .fov = 60};
    std::vector<Entity> mEntities;
    int mCurrentEntity = 0;

    // Old
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    bool framebufferResized = false;

public:
    void Run()
    {
        InitWindow();
        InitVulkan();
        InitImgui();
        LoadMeshes();
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
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void InitVulkan()
    {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateDevice();
        GetQueues();
        CreateAllocator();
        CreateSwapchain();
        CreateSyncObjects();
        CreateCommandObjects();
        InitializeDescriptors();
        InitializePipelines();

        //createDepthResources();
        //createTextureImage();
        //createTextureImageView();
        //createTextureSampler();
        //createUniformBuffers();
    }

    void InitImgui();

    void MainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // if (ImGui::Begin("background"))
            // {
            //     ComputeEffect& selected = backgroundEffects[currentBackgroundEffect];
            //
            //     ImGui::Text("Selected effect: ", selected.name);
            //
            //     ImGui::SliderInt("Effect Index", &currentBackgroundEffect,0, backgroundEffects.size() - 1);
            //
            //     ImGui::InputFloat4("data1",(float*)& selected.data.data1);
            //     ImGui::InputFloat4("data2",(float*)& selected.data.data2);
            //     ImGui::InputFloat4("data3",(float*)& selected.data.data3);
            //     ImGui::InputFloat4("data4",(float*)& selected.data.data4);
            // }
            // ImGui::End();

            if (ImGui::Begin("Transform"))
            {
                Entity& selected = mEntities[mCurrentEntity];

                ImGui::SliderInt("Entity Index", &mCurrentEntity,0, mEntities.size() - 1);


                static char buffer[64]{};
                assert(selected.name.size() < 64);
                selected.name.copy(buffer, selected.name.size());
                buffer[selected.name.size()] = '\0';

                if (ImGui::InputText("Name: ", buffer, IM_ARRAYSIZE(buffer)))
                {
                    selected.name = std::string(buffer);
                }

                ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&selected.position));
                ImGui::InputFloat3("Rotation:", reinterpret_cast<float *>(&selected.rotation));
                ImGui::InputFloat("Scale", &selected.scale);
            }
            ImGui::End();

            if (ImGui::Begin("Camera"))
            {
                ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&mCamera.position));
                ImGui::InputFloat("FOV", &mCamera.fov);
            }
            ImGui::End();
            
            ImGui::Render();

            Draw();
        }

        vkDeviceWaitIdle(mDevice);
    }

    void Cleanup()
    {
        glfwTerminate();
    }

    void LoadMeshes();

    void RecreateSwapchain();
    void CleanupSwapchain();

    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateDevice();
    void GetQueues();
    void CreateAllocator();
    void CreateSwapchain();
    void CreateCommandObjects();
    void CreateSyncObjects();
    void InitializeDescriptors();

    // Old
    void createDepthResources();
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void createUniformBuffers();

    // Drawing
    void Draw();
    void DrawBackground(VkCommandBuffer pCmd, u32 pImageIndex);
    void DrawImgui(VkCommandBuffer pCmd, VkImageView pTargetImageView);
    void DrawGeometry(VkCommandBuffer pCmd);

    // Pipelines
    void InitializePipelines();
    void InitializeBackgroundPipeline();
    void InitializeMeshPipeline();

    FrameData& GetCurrentFrame() { return mFrames[mCurrentFrame % MAX_FRAMES_IN_FLIGHT]; }

    void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& pFunction);

public:
    MeshBuffers UploadMesh(std::span<u32> pIndices, std::span<Vertex> pVertices);

private:
    // Old
    u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void updateUniformBuffer(u32 currentImage);
    void copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);

    VkCommandBuffer BeginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
};
