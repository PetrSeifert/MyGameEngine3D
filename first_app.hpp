#pragma once

#include <my_window.hpp>
#include <my_device.hpp>
#include <my_game_object.hpp>
#include <my_renderer.hpp>
#include <my_descriptors.hpp>

//std
#include <memory>

class FirstApp{
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp& operator=(const FirstApp &) = delete;
    
        void run();
    
    private:
        void loadGameObjects();

        MyWindow myWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        MyDevice myDevice{myWindow};
        MyRenderer myRenderer{myWindow, myDevice};

        std::unique_ptr<MyDescriptorPool> globalPool{};
        MyGameObject::Map gameObjects;   
};