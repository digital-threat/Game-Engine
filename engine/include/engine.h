#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vendor/stb/stb_image.h>
#include <VkBootstrap.h>

#include <vector>
#include <array>
#include <optional>
#include <fstream>

#include "types.h"
#include "vertex.h"

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

const std::vector<Vertex> vertices =
{
    {glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0, 0, 0), glm::vec2(0.0f, 0.0f)},
    {glm::vec3(0.5f, -0.5f, -0.5f),    glm::vec3(0, 0, 0), glm::vec2(1.0f, 0.0f)},
    {glm::vec3(0.5f, 0.5f, -0.5f),    glm::vec3(0, 0, 0), glm::vec2(1.0f, 1.0f)},
    {glm::vec3(-0.5f, 0.5f, -0.5f),   glm::vec3(0, 0, 0), glm::vec2(0.0f, 1.0f)},

    {glm::vec3(-0.5f, -0.5f, 0.5f),    glm::vec3(0, 0, 0), glm::vec2(0.0f, 0.0f)},
    {glm::vec3(0.5f, -0.5f, 0.5f),     glm::vec3(0, 0, 0), glm::vec2(1.0f, 0.0f)},
    {glm::vec3(0.5f, 0.5f, 0.5f),     glm::vec3(0, 0, 0), glm::vec2(1.0f, 1.0f)},
    {glm::vec3(-0.5f, 0.5f, 0.5f),    glm::vec3(0, 0, 0), glm::vec2(0.0f, 1.0f)},
};

const std::vector<u16> indices =
{
    4, 5, 6, 6, 7, 4, // Top
    0, 3, 2, 2, 1, 0, // Bottom
    0, 1, 5, 5, 4, 0, // Front
    1, 2, 6, 6, 5, 1, // Right
    2, 3, 7, 7, 6, 2, // Back
    3, 0, 4, 4, 7, 3  // Left
};

struct FrameData
{
    std::vector<VkSemaphore> swapchain_semaphores;
    std::vector<VkSemaphore> render_semaphores;
    std::vector<VkFence> in_flight_fences;

    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;
};

class Engine
{
private:
    GLFWwindow *window = nullptr;

    vkb::Instance m_vkb_instance;
    vkb::PhysicalDevice m_vkb_physical_device;
    vkb::Device m_vkb_device;
    vkb::Swapchain m_vkb_swapchain;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    FrameData m_frames[MAX_FRAMES_IN_FLIGHT]{};
    u32 current_frame = 0;

    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;

    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    std::vector<VkFramebuffer> m_swapchain_framebuffers;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    bool framebufferResized = false;

public:
    void run()
    {
        initWindow();
        InitVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow()
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
        CreateSwapchain();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        CreateCommandPoolAndAllocateBuffers();
        createDepthResources();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(m_device);
    }

    void cleanup()
    {
        glfwTerminate();
    }

    void RecreateSwapchain();
    void CleanupSwapchain();

    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateDevice();
    void GetQueues();
    void CreateSwapchain();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void CreateCommandPoolAndAllocateBuffers();
    void createDepthResources();
    void createFramebuffers();
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createSyncObjects();

    void drawFrame();

    FrameData& GetCurrentFrame() { return m_frames[current_frame % MAX_FRAMES_IN_FLIGHT]; }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void updateUniformBuffer(u32 currentImage);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    static std::vector<char> readFile(const std::string &filename);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
};
