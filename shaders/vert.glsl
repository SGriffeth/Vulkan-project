#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inWeights;
layout(location = 2) in uvec4 inBoneids;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform mvpUbo {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp_ubo;

layout(set = 1, binding = 0) uniform boneUbo {
    mat4 bones[100];
} bone_ubo;

void main() {
    gl_Position = vec4(inPosition, 1.0);
}