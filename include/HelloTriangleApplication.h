#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <optional>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <deque>
#include <functional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#ifndef SKELETAL_ANIMATION
#include <skeletalAnimation.h>
#define SKELETAL_ANIMATION
#endif
#include <descriptorManager.h>

#include <backends/imgui_impl_vulkan.h>


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}

		deletors.clear();
	}
};

class HelloTriangleApplication {
public:
    GLFWwindow* window;
    std::vector<VkDescriptorSet> mvpSets;
    std::vector<VkDescriptorSet> bonesSets;
    VkDescriptorPool imguiPool;
    VkFence _immFence;
    VkCommandBuffer _immCommandBuffer;
    VkCommandPool _immCommandPool;
    HelloTriangleApplication(GLFWcursorposfun mouse_callback, GLFWkeyfun key_callback, GLFWcursorenterfun cursor_enter_callback, GLFWframebuffersizefun frameBufferResizeCallback);
    HelloTriangleApplication();
    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
    void setCallbacks(GLFWcursorposfun mouse_callback, GLFWkeyfun key_callback, GLFWcursorenterfun cursor_enter_callback, GLFWframebuffersizefun frameBufferResizeCallback);
    void run();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkDevice device;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    DeletionQueue deletionQueue;
    bool framebufferResized = false;
    struct PlayerState {
        glm::vec3 cameraFront;
        glm::vec3 cameraPos;
        glm::vec3 cameraUp;
        glm::vec3 cameraRight;
        glm::vec3 worldUp = glm::vec3(0, 1, 0);
        float movementPerUpdate = 0.01f;
    } playerState;

private:
    GLFWcursorposfun mouse_callback;
    GLFWkeyfun key_callback;
    GLFWcursorenterfun cursor_enter_callback;
    GLFWframebuffersizefun  frameBufferResizeCallback;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    skeletalAnimation skelAnim = skeletalAnimation("../anims/pine/scene.gltf");
    descriptorManager descriptorMan;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    QueueFamilyIndices indices;
    uint imageCount;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    
    void initImGui();
    void processInput(GLFWwindow* window);
    void initWindow();
    void initVulkan();
    void mainLoop();
    void updateUbos();
    void cleanupSwapChain();
    void cleanup();
    void recreateSwapChain();
    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createCommandBuffers();
    void recordCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
 
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};