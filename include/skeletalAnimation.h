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
    glm::vec3 pos;
    glm::uvec4 boneids;
    glm::vec4 weights;

    void addBoneData(uint boneid, float weight) {
        for(uint i = 0; i < 4; i++) {
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

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, weights);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_UINT;
        attributeDescriptions[2].offset = offsetof(Vertex, boneids);

        return attributeDescriptions;
    }
};

class skeletalAnimation
{
public:
    skeletalAnimation(std::string name);
    ~skeletalAnimation();

    void traverse();

    struct Node {
        std::string name;
        aiMatrix4x4 transformation;
        std::vector<uint> meshes;
        int parent;
        int firstChild;
        int nextSibling;
    };

    struct Scene {
        Node root;
        std::vector<aiMatrix4x4> globalTransforms;
        std::vector<aiMatrix4x4> localTransforms;
        std::vector<Node> hierarchy;
        std::vector<Vertex> vertices;
        std::vector<uint> indices;
        std::map<std::string, uint> bone_name_to_index_map;
        struct BoneInfo
        {
            aiMatrix4x4 OffsetMatrix;
            aiMatrix4x4 FinalTransformation;

            BoneInfo(const aiMatrix4x4& Offset)
            {
                OffsetMatrix = Offset;
                FinalTransformation = aiMatrix4x4();
            }
        };

        std::vector<BoneInfo> bones;

        std::vector<uint> mesh_base_vertex;

        uint getBoneId(const aiBone* bone) {
            uint boneId = 0;
            std::string boneName(bone->mName.C_Str());

            if(bone_name_to_index_map.find(boneName) == bone_name_to_index_map.end()) {
                boneId = bone_name_to_index_map.size();
                bone_name_to_index_map[boneName] = boneId;
            } else {
                boneId = bone_name_to_index_map[boneName];
            }
            return boneId;
        }
    } scene;

private:
    /* data */
    Assimp::Importer importer;
    const aiScene* asmpScene;
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