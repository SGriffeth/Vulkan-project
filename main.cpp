#include <HelloTriangleApplication.h>
#ifndef SKELETAL_ANIMATION
#include <skeletalAnimation.h>
#define SKELETAL_ANIMATION
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

#include <chrono>

#include <MINEANARCHYConfig.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

HelloTriangleApplication app;

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    app.framebufferResized = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void cursor_enter_callback(GLFWwindow* window, int entered)
{
    if (entered)
    {
        // The cursor entered the content area of the window
        glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        // The cursor left the content area of the window
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static bool firstMouse = true;
    static float lastX, lastY;
    static float pitch = 0;
    static float yaw = -90.0f;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    app.playerState.cameraFront = glm::normalize(direction);
    app.playerState.cameraRight = glm::normalize(glm::cross(app.playerState.cameraFront, app.playerState.worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    app.playerState.cameraUp    = glm::normalize(glm::cross(app.playerState.cameraRight, app.playerState.cameraFront));
}

int main() {
    app.setCallbacks(mouse_callback, key_callback, cursor_enter_callback, framebufferResizeCallback);
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}