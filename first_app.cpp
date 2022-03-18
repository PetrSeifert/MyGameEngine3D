#include <first_app.hpp>

#include <my_camera.hpp>
#include <keyboard_movement_controller.hpp>
#include <my_buffer.hpp>
#include <systems/simple_renderer_system.hpp>
#include <systems/point_light_system.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <chrono>

struct GlobalUbo{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
    glm::vec3 lightPosition{-5.f, -6.f, 0.f};
    alignas(16) glm::vec4 lightColor{1.f};
};

FirstApp::FirstApp(){
    globalPool = MyDescriptorPool::Builder(myDevice)
        .setMaxSets(MySwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MySwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
    loadGameObjects();
}

FirstApp::~FirstApp(){}

void FirstApp::run() {

    std::vector<std::unique_ptr<MyBuffer>> uboBuffers(MySwapChain::MAX_FRAMES_IN_FLIGHT);
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

    auto globalSetLayout = MyDescriptorSetLayout::Builder(myDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(MySwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++){
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        MyDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

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
    MyCamera camera{};

    //camera.setViewTarget(glm::vec3(-1.f, -2.f, 11.3f), glm::vec3(0.f, 0.f, 1.5f));

    auto viewerObject = MyGameObject::createGameObject();
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!myWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        //set max frameTime for big lags

        cameraController.moveInPlaneXZ(myWindow.getGLFWWindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = myRenderer.getAspectRatio();
        //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);

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


void FirstApp::loadGameObjects(){
    std::shared_ptr<MyModel> myModel = MyModel::createModelFromFile(myDevice, "models/model.obj");
    auto gameObj = MyGameObject::createGameObject();
    gameObj.model = myModel;
    gameObj.transform.translation = {.0f, .0f, 0.f};
    gameObj.transform.scale = {1.f, 1.f, 1.f};
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));

    myModel = MyModel::createModelFromFile(myDevice, "models/floor.obj");
    auto floor = MyGameObject::createGameObject();
    floor.model = myModel;
    floor.transform.translation = {.0f, 0.f, 0.f};
    floor.transform.scale = {15.f, 1.f, 15.f};
    gameObjects.emplace(floor.getId(), std::move(floor));
}