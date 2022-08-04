#pragma once

#include <my_window.hpp>
#include <my_device.hpp>
#include <my_game_object.hpp>
#include <my_renderer.hpp>
#include <my_descriptors.hpp>
#include <keyboard_movement_controller.hpp>
#include <my_camera.hpp>
#include <my_buffer.hpp>

#include <systems/simple_renderer_system.hpp>
#include <systems/point_light_system.hpp>

//std
#include <memory>
#include <stdexcept>
#include <array>
#include <iostream>
#include <algorithm>
#include <chrono>

class App{
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        App();
        ~App();

        //App(const App &) = delete;
        //App& operator=(const App &) = delete;

        void setup();
        void run();
    
    private:
        void loadGameObjects();
        
        MyWindow myWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        MyDevice myDevice{myWindow};
        MyRenderer myRenderer{myWindow, myDevice};

        int cameraSize = 20;
        int fpsLock = 5;
        float minFrameTime = 0.f / fpsLock;
        float sumFrameTime = 0;
        float totalTime = 0.f;
        int cycles = 0;

        std::vector<std::unique_ptr<MyBuffer>> uboBuffers;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        MyGameObject viewerObject = MyGameObject::createGameObject();
        std::chrono::steady_clock::time_point currentTime;
        KeyboardMovementController cameraController;
        MyCamera camera{};


        std::unique_ptr<MyDescriptorSetLayout> globalSetLayout = MyDescriptorSetLayout::Builder(myDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

        SimpleRendererSystem simpleRendererSystem{
            myDevice, 
            myRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            myDevice, 
            myRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout()
        };

        std::unique_ptr<MyDescriptorPool> globalPool{};
        MyGameObject::Map gameObjects;  
};