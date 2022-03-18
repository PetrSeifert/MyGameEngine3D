#include <my_renderer.hpp>

//std
#include <stdexcept>
#include <array>

MyRenderer::MyRenderer(MyWindow& window, MyDevice& device) : myWindow{window}, myDevice{device}{
    //bug??
    // currentFrameIndex = 0;
    recreateSwapChain();
    createCommandBuffers();
}

MyRenderer::~MyRenderer(){ freeCommandBuffers(); }


void MyRenderer::recreateSwapChain(){
    auto extent = myWindow.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = myWindow.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(myDevice.device());
    //mySwapChain = nullptr;

    if (mySwapChain == nullptr){
        mySwapChain = std::make_unique<MySwapChain>(myDevice, extent);
    }else{
        std::shared_ptr<MySwapChain> oldSwapChain = std::move(mySwapChain);
        mySwapChain = std::make_unique<MySwapChain>(myDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*mySwapChain.get())){
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }

    // back
}

void MyRenderer::createCommandBuffers(){
    commandBuffers.resize(MySwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = myDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(myDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("failed to allocate command buffers");
    }
}

void MyRenderer::freeCommandBuffers(){
    vkFreeCommandBuffers(
        myDevice.device(),
        myDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    
    commandBuffers.clear();
}

VkCommandBuffer MyRenderer::beginFrame(){
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = mySwapChain->acquireNextImage(&currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("failed to acquire swap chain image");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("failed to begin recording command buffer");
    }
    return commandBuffer;
}

void MyRenderer::endFrame(){
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("failed to record command buffer");
    }

    auto result = mySwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myWindow.wasWindowResized()){
        myWindow.resetWindowResizedFlag();
        recreateSwapChain();
    } else if (result != VK_SUCCESS){
        throw std::runtime_error("failed to present swap chain image");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % MySwapChain::MAX_FRAMES_IN_FLIGHT;
}

void MyRenderer::beginSwapChainRendererPass(VkCommandBuffer commandBuffer){
    assert(isFrameStarted && "Can't call beginSwapChainRendererPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame"); 
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mySwapChain->getRenderPass();
    renderPassInfo.framebuffer = mySwapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mySwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mySwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(mySwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, mySwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void MyRenderer::endSwapChainRendererPass(VkCommandBuffer commandBuffer){
    assert(isFrameStarted && "Can't call endSwapChainRendererPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame"); 
    
    vkCmdEndRenderPass(commandBuffer);
}
