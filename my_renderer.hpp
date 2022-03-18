#pragma once

#include <my_window.hpp>
#include <my_device.hpp>
#include <my_swap_chain.hpp>
#include <my_model.hpp>

//std
#include <memory>
#include <cassert>
#include <vector>

class MyRenderer{
    public:

        MyRenderer(MyWindow& window, MyDevice& device);
        ~MyRenderer();

        MyRenderer(const MyRenderer &) = delete;
        MyRenderer& operator=(const MyRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return mySwapChain->getRenderPass(); }
        float getAspectRatio() const { return mySwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const { 
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex]; 
        }

        int getFrameIndex() const { 
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex; 
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRendererPass(VkCommandBuffer commandBuffer);
        void endSwapChainRendererPass(VkCommandBuffer commandBuffer);
    
    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();

        MyWindow& myWindow;
        MyDevice& myDevice;
        std::unique_ptr<MySwapChain> mySwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex{0};
        int currentFrameIndex = 0;
        bool isFrameStarted{false};
};