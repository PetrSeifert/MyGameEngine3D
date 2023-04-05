#pragma once

#include <my_camera.hpp>
#include <my_game_object.hpp>

#include <vulkan/vulkan.h>

struct FrameInfo{
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    MyCamera &camera;
    VkDescriptorSet globalDescriptorSet;
    MyGameObject::Map &gameObjects;
};