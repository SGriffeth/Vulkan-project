#include <glm/glm.hpp>
#include <map>
#include <array>
#include <vector>
#include <stack>
#include <iostream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

struct Vertex {
    static const uint max_bones_in_num_of_vec4 = 4;
    glm::vec3 pos;
    uint boneids[max_bones_in_num_of_vec4*4];
    float weights[max_bones_in_num_of_vec4*4];

    void addBoneData(uint boneid, float weight) {
        for(uint i = 0; i < max_bones_in_num_of_vec4*4; i++) {
            if(weights[i] == 0) {
                boneids[i] = boneid;
                weights[i] = weight;
                return;
            }
        }

        throw std::runtime_error("More bones than we have space for");
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.resize(max_bones_in_num_of_vec4*2+1);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        uint i = 1;
        for(; i < max_bones_in_num_of_vec4+1; i++) {
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].location = i;
            attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[i].offset = offsetof(Vertex, weights) + (i-1)*16;
        }

        for(uint i2 = 0; i2 < max_bones_in_num_of_vec4; i2++) {
            attributeDescriptions[i2+i].binding = 0;
            attributeDescriptions[i2+i].location = i2+i;
            attributeDescriptions[i2+i].format = VK_FORMAT_R32G32B32A32_UINT;
            attributeDescriptions[i2+i].offset = offsetof(Vertex, boneids) + i2*16;
        }

        return attributeDescriptions;
    }
};

class skeletalAnimation
{
public:
    const aiScene* asmpScene;
    skeletalAnimation(std::string name);
    ~skeletalAnimation();

    void loadAnimation(std::string path);
    void traverse(float animationTime);

    struct Node {
        std::string name;
        aiMatrix4x4 transformation;
        int parent;
        int firstChild;
        int nextSibling;
    };

    struct Scene {
        Node root;
        int animationIndex = -1;
        int animationDuration;
        aiMatrix4x4 globalInverseTransform;
        std::vector<Node> hierarchy;
        std::vector<Vertex> vertices;
        std::vector<uint> indices;
        std::map<std::string, uint> bone_name_to_index_map;
        struct BoneInfo
        {
            aiMatrix4x4 OffsetMatrix;
            //int FinalTransformation;

            BoneInfo(const aiMatrix4x4& Offset)
            {
                OffsetMatrix = Offset;
                //FinalTransformation = -1;
            }
        };

        std::vector<aiMatrix4x4> finalTransforms;
        std::vector<BoneInfo> bones;

        std::vector<uint> mesh_base_vertex;

        uint getBoneId(const aiBone* bone) {
            uint boneId = 0;
            std::string boneName(bone->mName.C_Str());

            if(bone_name_to_index_map.find(boneName) == bone_name_to_index_map.end()) {
                boneId = bone_name_to_index_map.size();
                bone_name_to_index_map[boneName] = boneId;
                bones.push_back(BoneInfo(bone->mOffsetMatrix));
                finalTransforms.push_back(aiMatrix4x4());
            } else {
                boneId = bone_name_to_index_map[boneName];
            }
            return boneId;
        }
    } scene;

private:
    /* data */
    Assimp::Importer importer;
    aiNodeAnim* findNodeAnim(const char* nodeName);
    void convertScene();
    void loadMeshes();
    void loadBone(const aiBone* bone, uint meshIndex);
    uint FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);

    uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

    uint FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
};