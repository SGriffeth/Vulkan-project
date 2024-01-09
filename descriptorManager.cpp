#include <stdexcept>
#include <HelloTriangleApplication.h>
#include <cstring>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

descriptorManager::descriptorManager() {
    layouts.resize(2);
}

void descriptorManager::allocateUbos(HelloTriangleApplication& app, uint boneCount) {
    VkDeviceSize mvpSize = sizeof(mvpBuffer);
    VkDeviceSize boneSize = sizeof(aiMatrix4x4)*boneCount;

    mvpUbos.resize(app.MAX_FRAMES_IN_FLIGHT);
    boneUbos.resize(app.MAX_FRAMES_IN_FLIGHT);

    for(int i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
        app.createBuffer(mvpSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         mvpUbos[i].buffer, mvpUbos[i].bufferMemory);
         vkMapMemory(app.device, mvpUbos[i].bufferMemory, 0, mvpSize, 0, &mvpUbos[i].bufferMapped);
         app.createBuffer(boneSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         boneUbos[i].buffer, boneUbos[i].bufferMemory);
         vkMapMemory(app.device, boneUbos[i].bufferMemory, 0, boneSize, 0, &boneUbos[i].bufferMapped);
        app.deletionQueue.push_function([=, &app]() {
            vkDestroyBuffer(app.device, boneUbos[i].buffer, nullptr);
            vkFreeMemory(app.device, boneUbos[i].bufferMemory, nullptr);
            vkDestroyBuffer(app.device, mvpUbos[i].buffer, nullptr);
            vkFreeMemory(app.device, mvpUbos[i].bufferMemory, nullptr);
        });
    }
}

void descriptorManager::createDescriptorSets(HelloTriangleApplication& app, std::vector<VkDescriptorSet>& mvpSets, std::vector<VkDescriptorSet>& bonesSets, uint boneCount) {
    mvpSets.resize(app.MAX_FRAMES_IN_FLIGHT);
    bonesSets.resize(app.MAX_FRAMES_IN_FLIGHT);
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = app.MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = &poolSize;
    createInfo.maxSets = app.MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(app.device, &createInfo, nullptr, &mvpPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create mvp descriptor pool!");
    }

    VkDescriptorPoolSize bonesPoolSize = {};
    bonesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bonesPoolSize.descriptorCount = app.MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo bonesCreateInfo = {};
    bonesCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    bonesCreateInfo.poolSizeCount = 1;
    bonesCreateInfo.pPoolSizes = &bonesPoolSize;
    bonesCreateInfo.maxSets = app.MAX_FRAMES_IN_FLIGHT;

    if(vkCreateDescriptorPool(app.device, &bonesCreateInfo, nullptr, &bonesPool)) {
        throw std::runtime_error("failed to create bones descriptor pool!");
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    std::vector<VkDescriptorSetLayout> bonesLayouts(app.MAX_FRAMES_IN_FLIGHT, layouts[0]);
    std::vector<VkDescriptorSetLayout> mvpLayouts(app.MAX_FRAMES_IN_FLIGHT, layouts[1]);
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mvpPool;
    allocInfo.descriptorSetCount = app.MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = mvpLayouts.data();
    if(vkAllocateDescriptorSets(app.device, &allocInfo, mvpSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate mvp descriptor sets");
    }

    VkDescriptorSetAllocateInfo bonesAlloc = {};
    bonesAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    bonesAlloc.descriptorPool = bonesPool;
    bonesAlloc.descriptorSetCount = app.MAX_FRAMES_IN_FLIGHT;
    bonesAlloc.pSetLayouts = bonesLayouts.data();
    if(vkAllocateDescriptorSets(app.device, &bonesAlloc, bonesSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate bones descriptor sets");
    }

    app.deletionQueue.push_function([&]() {
            vkDestroyDescriptorPool(app.device, mvpPool, nullptr);
            vkDestroyDescriptorPool(app.device, bonesPool, nullptr);
    });

    allocateUbos(app, boneCount);

    for(unsigned int i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo mvpBufferInfo = {};
        mvpBufferInfo.buffer = mvpUbos[i].buffer;
        mvpBufferInfo.offset = 0;
        mvpBufferInfo.range = sizeof(mvpBuffer);

        VkDescriptorBufferInfo bonesBufferInfo = {};
        bonesBufferInfo.buffer = boneUbos[i].buffer;
        bonesBufferInfo.offset = 0;
        bonesBufferInfo.range = sizeof(aiMatrix4x4)*boneCount;

        VkWriteDescriptorSet mvpDescriptorWrite{};
        mvpDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpDescriptorWrite.dstSet = mvpSets[i];
        mvpDescriptorWrite.dstBinding = 0;
        mvpDescriptorWrite.dstArrayElement = 0;
        mvpDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpDescriptorWrite.descriptorCount = 1;
        mvpDescriptorWrite.pBufferInfo = &mvpBufferInfo;

        VkWriteDescriptorSet bonesDescriptorWrite{};
        bonesDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bonesDescriptorWrite.dstSet = bonesSets[i];
        bonesDescriptorWrite.dstBinding = 0;
        bonesDescriptorWrite.dstArrayElement = 0;
        bonesDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bonesDescriptorWrite.descriptorCount = 1;
        bonesDescriptorWrite.pBufferInfo = &bonesBufferInfo;

        VkWriteDescriptorSet descriptorWrites[] = {bonesDescriptorWrite, mvpDescriptorWrite};

        vkUpdateDescriptorSets(app.device, 2, descriptorWrites, 0, nullptr);
    }
}

void descriptorManager::updateDescriptorSets(HelloTriangleApplication& app, std::vector<VkDescriptorSet>& mvpSets, std::vector<VkDescriptorSet>& bonesSets, uint boneCount) {
    for(unsigned int i = 0; i < app.MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo mvpBufferInfo = {};
        mvpBufferInfo.buffer = mvpUbos[i].buffer;
        mvpBufferInfo.offset = 0;
        mvpBufferInfo.range = sizeof(mvpBuffer);

        VkDescriptorBufferInfo bonesBufferInfo = {};
        bonesBufferInfo.buffer = boneUbos[i].buffer;
        bonesBufferInfo.offset = 0;
        bonesBufferInfo.range = sizeof(aiMatrix4x4)*boneCount;

        VkWriteDescriptorSet mvpDescriptorWrite{};
        mvpDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpDescriptorWrite.dstSet = mvpSets[i];
        mvpDescriptorWrite.dstBinding = 0;
        mvpDescriptorWrite.dstArrayElement = 0;
        mvpDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpDescriptorWrite.descriptorCount = 1;
        mvpDescriptorWrite.pBufferInfo = &mvpBufferInfo;

        VkWriteDescriptorSet bonesDescriptorWrite{};
        bonesDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bonesDescriptorWrite.dstSet = bonesSets[i];
        bonesDescriptorWrite.dstBinding = 0;
        bonesDescriptorWrite.dstArrayElement = 0;
        bonesDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bonesDescriptorWrite.descriptorCount = 1;
        bonesDescriptorWrite.pBufferInfo = &bonesBufferInfo;

        VkWriteDescriptorSet descriptorWrites[] = {bonesDescriptorWrite, mvpDescriptorWrite};

        vkUpdateDescriptorSets(app.device, 2, descriptorWrites, 0, nullptr);
    }
}

VkDescriptorSetLayout* descriptorManager::createDescriptorSetLayouts(HelloTriangleApplication& app) {
    VkDescriptorSetLayoutBinding mvpBinding = {};
    mvpBinding.binding = 0;
    mvpBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mvpBinding.descriptorCount = 1;
    mvpBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    mvpBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bonesBinding = {};
    bonesBinding.binding = 0;
    bonesBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bonesBinding.descriptorCount = 1;
    bonesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bonesBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.bindingCount = 1;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pBindings = &mvpBinding;

    std::cout << "???" << std::endl;
    std::cout << "layouts.data(): " << &layouts.at(0) << std::endl;
    if (vkCreateDescriptorSetLayout(app.device, &createInfo, nullptr, layouts.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create mvp descriptor set layout!");
    }

    VkDescriptorSetLayoutCreateInfo bonesCreateInfo = {};
    bonesCreateInfo.bindingCount = 1;
    bonesCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    bonesCreateInfo.pBindings = &bonesBinding;

    std::cout << "layouts.data()+1: " << &layouts.at(1) << std::endl;
    if(vkCreateDescriptorSetLayout(app.device, &bonesCreateInfo, nullptr, layouts.data()+1) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bones descriptor set layout");
    }

    app.deletionQueue.push_function([&]() {
        vkDestroyDescriptorSetLayout(app.device, layouts[0], nullptr);
        vkDestroyDescriptorSetLayout(app.device, layouts[1], nullptr);
    });

    return layouts.data();
}

void descriptorManager::updateUbos(std::vector<aiMatrix4x4>& bones, mvpBuffer mvp, uint currentImage) {
    memcpy(boneUbos[currentImage].bufferMapped, bones.data(), bones.size()*sizeof(aiMatrix4x4));
    memcpy(mvpUbos[currentImage].bufferMapped, &mvp, sizeof(mvpBuffer));
}