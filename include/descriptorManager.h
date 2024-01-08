/*#ifndef HELLO_TRIANGLE_APP
#include <HelloTriangleApplication.h>
#define HELLO_TRIANGLE_APP
#endif*/
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <assimp/scene.h>

class HelloTriangleApplication;

class descriptorManager
{
private:
    /* data */
    std::vector<VkDescriptorSetLayout> layouts;
    VkDescriptorPool mvpPool;
    VkDescriptorPool bonesPool;
    struct uboBuffer {
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
        void* bufferMapped;
    };
    std::vector<uboBuffer> mvpUbos;
    std::vector<uboBuffer> boneUbos;
    
    void allocateUbos(HelloTriangleApplication& app);
public:
    descriptorManager();
    static const unsigned int BONES_COUNT = 100;
    struct boneBuffer {
        aiMatrix4x4 bones[descriptorManager::BONES_COUNT];
    };

    struct mvpBuffer {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    VkDescriptorSetLayout* createDescriptorSetLayouts(HelloTriangleApplication& app);
    void createDescriptorSets(HelloTriangleApplication& app, std::vector<VkDescriptorSet>& mvpSets, std::vector<VkDescriptorSet>& bonesSets);
    void updateUbos(boneBuffer, mvpBuffer, uint);
};