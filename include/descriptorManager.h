#ifndef HELLO_TRIANGLE_APP
#include <HelloTriangleApplication.h>
#define HELLO_TRIANGLE_APP
#endif
#include <assimp/scene.h>
#include <glm/glm.hpp>

class descriptorManager
{
private:
    /* data */
    VkDescriptorSetLayout layouts[2];
    VkDescriptorPool mvpPool;
    VkDescriptorPool bonesPool;
    struct uboBuffer {
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
        void* bufferMapped;
    };
    std::vector<uboBuffer> mvpUbos;
    std::vector<uboBuffer> boneUbos;
    
    HelloTriangleApplication* app;
    void allocateUbos();
public:
    static const unsigned int BONES_COUNT = 100;
    struct boneBuffer {
        aiMatrix4x4 bones[descriptorManager::BONES_COUNT];
    };

    struct mvpBuffer {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    descriptorManager(HelloTriangleApplication* app);

    VkDescriptorSetLayout* createDescriptorSetLayouts();
    void createDescriptorSets(std::vector<VkDescriptorSet>& mvpSets, std::vector<VkDescriptorSet>& bonesSets);
    void updateUbos(boneBuffer, mvpBuffer, uint);
};