#version 450

const uint max_bones_in_num_of_vec4 = 4;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inWeights[max_bones_in_num_of_vec4];
layout(location = max_bones_in_num_of_vec4+1) in uvec4 inBoneids[max_bones_in_num_of_vec4];

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform mvpUbo {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp_ubo;

layout(set = 1, binding = 0) uniform boneUbo {
    mat4 bones[118];
} bone_ubo;

void main() {
    mat4 boneTransform = mat4(0);
    for(uint i = 0; i < max_bones_in_num_of_vec4; i++) {
        boneTransform += bone_ubo.bones[inBoneids[i].x] * inWeights[i].x;
        boneTransform += bone_ubo.bones[inBoneids[i].y] * inWeights[i].y;
        boneTransform += bone_ubo.bones[inBoneids[i].z] * inWeights[i].z;
        boneTransform += bone_ubo.bones[inBoneids[i].w] * inWeights[i].w;
    }
    gl_Position = mvp_ubo.proj * mvp_ubo.view * mvp_ubo.model * boneTransform * vec4(inPosition, 1.0);
}