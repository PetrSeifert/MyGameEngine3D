#include <sorting_algorithms/app.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct GlobalUbo{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
    glm::vec3 lightPosition{-5.f, -6.f, 0.f};
    alignas(16) glm::vec4 lightColor{1.f};
};

MyModel::Builder boardSprite = {{
    {{-1, -1, 0.f}, {0.01f, .01f, 0.01f}},
    {{1, -1, 0.f}, {0.01f, .01f, 0.01f}},
    {{1, 1, 0.f}, {0.01f, .01f, 0.01f}},
    {{-1, 1, 0.f}, {0.01f, .01f, 0.01f}}},
    {{0}, {2}, {3}, {0}, {1}, {2}}
};

MyModel::Builder greenSprite = {{
    {{-1, -1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{1, -1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{1, 1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{-1, 1, 0.f}, {0.0f, 1.0f, 0.0f}}},
    {{0}, {2}, {3}, {0}, {1}, {2}}
};

MyModel::Builder redSprite = {{
    {{-1, -1, 0.f}, {1.0f, 0.0f, 0.0f}},
    {{1, -1, 0.f}, {1.0f, 0.0f, 0.0f}},
    {{1, 1, 0.f}, {1.0f, 0.0f, 0.0f}},
    {{-1, 1, 0.f}, {1.0f, 0.0f, 0.0f}}},
    {{0}, {2}, {3}, {0}, {1}, {2}}
};

App::App(){
    globalPool = MyDescriptorPool::Builder(myDevice)
        .setMaxSets(MySwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MySwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    uboBuffers = std::vector<std::unique_ptr<MyBuffer>>(MySwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++){
        uboBuffers[i] = std::make_unique<MyBuffer>(
            myDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->map();
    }

    globalDescriptorSets = std::vector<VkDescriptorSet>(MySwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++){
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        MyDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }
    
    setup();
    currentTime = std::chrono::high_resolution_clock::now();
}

App::~App(){}

void App::setup(){ 
    loadGameObjects();
}

void App::run() {
    while (!myWindow.shouldClose()){
        glfwPollEvents();
        cycles++;
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        sumFrameTime += frameTime;
        totalTime += frameTime;

        if (!bubbleSort.sorted)
            bubbleSort.sortStep();

        for (int i = 0; i < bubbleSort.listSize; i++){
            int id = i + 1;
            auto it = std::find(bubbleSort.v.begin(),bubbleSort.v.end(), gameObjects.at(id).transform.scale.y);

            if (std::distance(bubbleSort.v.begin(), it) == bubbleSort.currentIndex || std::distance(bubbleSort.v.begin(), it) == bubbleSort.currentIndex + 1){
                std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, redSprite);
                gameObjects.at(id).model = myModel;
            }
            if (std::distance(bubbleSort.v.begin(), it) == bubbleSort.currentIndex - 1){
                std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, greenSprite);
                gameObjects.at(id).model = myModel;
            }
            if (bubbleSort.v.at(bubbleSort.listSize - 1) == gameObjects.at(id).transform.scale.y){
                std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, greenSprite);
                gameObjects.at(id).model = myModel;
            }

            it = std::find(bubbleSort.v.begin(),bubbleSort.v.end(), gameObjects.at(id).transform.scale.y);
            gameObjects.at(id).transform.translation = 
            {-cameraSize + 0.5f + std::distance(bubbleSort.v.begin(), it), 
             gameObjects.at(id).transform.translation.y, gameObjects.at(id).transform.translation.z};
        }


        //set max frameTime for big lags

        // cameraController.moveInPlaneXZ(myWindow.getGLFWWindow(), frameTime, viewerObject);

        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = myRenderer.getAspectRatio();
        camera.setOrthographicProjection(-aspect * cameraSize, aspect * cameraSize, -cameraSize, cameraSize, -1, 1);
        //camera.setPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);

        if (auto commandBuffer = myRenderer.beginFrame()){
            int frameIndex = myRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects
            };

            //update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            //render
            myRenderer.beginSwapChainRendererPass(commandBuffer);
            simpleRendererSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);
            myRenderer.endSwapChainRendererPass(commandBuffer);
            myRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(myDevice.device());
}

void App::loadGameObjects(){
    std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, greenSprite);
    std::cout << bubbleSort.listSize <<std::endl;
    for (int i = 0; i < bubbleSort.listSize; i++){
        std::cout << bubbleSort.v.at(i) << std::endl;
        auto gameObj = MyGameObject::createGameObject();
        gameObj.model = myModel;
        gameObj.transform.scale = {1, bubbleSort.v.at(i), 1};
        gameObj.transform.translation = {-cameraSize + 0.5f + i, cameraSize - gameObj.transform.scale.y / 2, 0};
        gameObj.color = {1, 0, 0};
        gameObjects.emplace(gameObj.getId(), std::move(gameObj));
    }

    myModel = std::make_shared<MyModel>(myDevice, boardSprite);
    auto gameObj = MyGameObject::createGameObject();
    gameObj.model = myModel;
    gameObj.transform.translation = {0, 0, .2f};
    gameObj.transform.scale = {cameraSize*2, cameraSize*2, cameraSize*2};
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));
}