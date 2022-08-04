#include <snake_ai/app.hpp>



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

MyModel::Builder snakeSprite = {{
    {{-1, -1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{1, -1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{1, 1, 0.f}, {0.0f, 1.0f, 0.0f}},
    {{-1, 1, 0.f}, {0.0f, 1.0f, 0.0f}}},
    {{0}, {2}, {3}, {0}, {1}, {2}}
};

MyModel::Builder foodSprite = {{
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
    loadGameObjects();
    setup();
}

App::~App(){}

void App::setup(){ 
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

    //camera.setViewTarget(glm::vec3(-1.f, -2.f, 11.3f), glm::vec3(0.f, 0.f, 1.5f));
    
    currentTime = std::chrono::high_resolution_clock::now();
    agent.model.load();
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


        //set max frameTime for big lags

        // cameraController.moveInPlaneXZ(myWindow.getGLFWWindow(), frameTime, viewerObject);
        moveDir = cameraController.getMoveDirection(myWindow.getGLFWWindow());
        if (moveDir == glm::vec3(0, -1, 0))
        {
            minFrameTime = 0.f / fpsLock;
        }
        else if (moveDir == glm::vec3(0, 1, 0))
        {
            minFrameTime = 1.f / fpsLock;
        }
        else if (moveDir == glm::vec3(1, 0, 0))
        {
            showPlot = true;
        }
        
        if (sumFrameTime >= minFrameTime)
        { 
            steps++;
            //std::cout << 1/(sumFrameTime/cycles) << std::endl;
            //test();
            train();
            if (game.snake_size > snakeBlocks.size()){
                std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, snakeSprite);
                auto gameObj = MyGameObject::createGameObject();
                gameObj.model = myModel;
                gameObj.transform.translation = {game.snake[0].x - cameraSize + 0.5f, game.snake[0].y - cameraSize + 0.5f, 0};
                gameObjects.emplace(gameObj.getId(), std::move(gameObj));
                snakeBlocks.insert(snakeBlocks.begin(), gameObj.getId());
                gameObjects.at(foodId).transform.translation = {game.food_x - cameraSize + 0.5f, game.food_y - cameraSize + 0.5f, .1f};
            }
            else{
                gameObjects.at(snakeBlocks[snakeBlocks.size() - 1]).transform.translation = {game.snake[0].x - cameraSize + 0.5f, game.snake[0].y - cameraSize + 0.5f, 0};
                std::rotate(snakeBlocks.begin(), snakeBlocks.end() - 1, snakeBlocks.end());
            }
            // if (!isDead(gameObjects.at(snakeBlocks[0]).transform.translation))
            // {
            //     move();
            //     if (canEat()) eat();
            // }
            if (game.isRestarted){
                restart();
                game.isRestarted = false;
            }
            cycles = 0;
            sumFrameTime = 0;
        }

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

void App::restart(){
    while(snakeBlocks.size() > game.snake_size){
        gameObjects.erase(snakeBlocks[snakeBlocks.size()-1]);
        snakeBlocks.pop_back();
    }
    for (int i = 0; i < game.snake_size; i++){
        gameObjects.at(snakeBlocks[i]).transform.translation = {game.snake[i].x - cameraSize + 0.5f, game.snake[i].y - cameraSize + 0.5f, 0};
    }
    gameObjects.at(foodId).transform.translation = {game.food_x - cameraSize + 0.5f, game.food_y - cameraSize + 0.5f, .1f};
}

void App::move(){
    if (moveDir != glm::vec3(0, 0, 0))
    {
        MyGameObject& currHead = gameObjects.at(snakeBlocks[0]);
        MyGameObject& newHead = gameObjects.at(snakeBlocks[snakeBlocks.size() - 1]);
        newHead.transform.translation = currHead.transform.translation + moveDir;
        std::rotate(snakeBlocks.begin(), snakeBlocks.end() - 1, snakeBlocks.end());
    }
}

bool App::isDead(glm::vec3 headPos){
    if (headPos.x >= cameraSize || 
        headPos.y >= cameraSize ||
        headPos.x <= -cameraSize || 
        headPos.y <= -cameraSize)
        return true;
    
    for (int i = 1; i < snakeBlocks.size(); i++)
    {
        if (headPos == gameObjects.at(snakeBlocks[i]).transform.translation)
            return true;
    }

    return false;
}

bool App::canEat(){
    MyGameObject *head = &gameObjects.at(snakeBlocks[0]);
    MyGameObject *food = &gameObjects.at(foodId);

    glm::vec3 headPos = head->transform.translation;
    glm::vec3 foodPos = food->transform.translation;
    if (headPos.x == foodPos.x && headPos.y == foodPos.y)
        return true;
    return false;
}

void App::eat(){
    gameObjects.at(foodId).transform.translation = getRandomPosition();
    std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, snakeSprite);
    auto gameObj = MyGameObject::createGameObject();
    gameObj.model = myModel;
    gameObj.transform.translation = gameObjects.at(snakeBlocks[snakeBlocks.size() - 1]).transform.translation;
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));
    snakeBlocks.insert(snakeBlocks.end(), gameObj.getId());
}

glm::vec3 App::getRandomPosition(){
    static std::random_device rd;  
    static std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(-cameraSize, cameraSize - 1);
    
    return glm::vec3(distrib(gen) + 0.5f, distrib(gen) + 0.5f, .1f);
}

void App::loadGameObjects(){
    std::shared_ptr<MyModel> myModel = std::make_shared<MyModel>(myDevice, snakeSprite);
    for (int i = 0; i < snakeLength; i++)
    {
        auto gameObj = MyGameObject::createGameObject();
        gameObj.model = myModel;
        // gameObj.transform.translation = {.5f + i, .5f, 0.f};
        // gameObjects.emplace(gameObj.getId(), std::move(gameObj));
        // snakeBlocks.insert(snakeBlocks.begin(), gameObj.getId());
        // gameObj.transform.translation = {cameraSize + 0.5f, cameraSize + 0.5f, 0};
        gameObj.transform.translation = {game.snake[i].x - cameraSize + 0.5f, game.snake[i].y - cameraSize + 0.5f, 0};
        gameObjects.emplace(gameObj.getId(), std::move(gameObj));
        snakeBlocks.insert(snakeBlocks.end(), gameObj.getId());
    }

    myModel = std::make_shared<MyModel>(myDevice, foodSprite);
    auto gameObj = MyGameObject::createGameObject();
    gameObj.model = myModel;
    gameObj.transform.translation = {game.food_x - cameraSize + 0.5f, game.food_y - cameraSize + 0.5f, .1f};
    // gameObj.transform.translation = {cameraSize + 0.5f, cameraSize + 0.5f, .1f};
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));
    foodId = gameObj.getId();

    myModel = std::make_shared<MyModel>(myDevice, boardSprite);
    gameObj = MyGameObject::createGameObject();
    gameObj.model = myModel;
    gameObj.transform.translation = {0, 0, .2f};
    gameObj.transform.scale = {cameraSize*2, cameraSize*2, cameraSize*2};
    gameObjects.emplace(gameObj.getId(), std::move(gameObj));
}

void App::train() {
    // get old state
    std::vector<float> state_old = agent.get_state(game);

    // get move
    std::vector<float> final_move = agent.get_action(state_old);

    // perform move and get new state
    std::vector<float> reward;
    std::vector<float> done;
    int score;
    tie(reward, done, score) = game.play_step(final_move);
    std::vector<float> state_new = agent.get_state(game);

    // train short memory
    agent.train_short_memory(state_old, final_move, reward, state_new, done);

    // remember
    agent.remember(state_old, final_move, reward, state_new, done);

    if (done[0] == 1) {
        // train long memory, plot result
        game.reset();
        agent.n_games += 1;
        agent.train_long_memory();

        if (score > record) {
            record = score;
            agent.model.save();
        }

        std::cout << "Game " << agent.n_games << " Steps " << steps << " Score " << score << " Record: " << record << std::endl;

        

        plot_scores.push_back(score);
        total_score += score;
        float mean_score = (float)total_score / agent.n_games;
        plot_mean_scores.push_back(mean_score);
        if (showPlot)
        {
            /* plt::clf();
            plt::title("Training...");
            plt::xlabel("Number of Games");
            plt::ylabel("Score");
            plt::plot(plot_scores);
            plt::plot(plot_mean_scores);
            plt::show(true); */
            showPlot = false;
        }
    }
}

void App::test() {
    // get old state
    std::vector<float> state_old = agent.get_state(game);
    // get move
    std::vector<float> final_move = agent.get_action(state_old);

    // perform move and get new state
    std::vector<float> reward;
    std::vector<float> done;
    int score;
    tie(reward, done, score) = game.play_step(final_move);

    if (done[0] == 1) {
        // train long memory, plot result
        game.reset();
        agent.n_games += 1;
        
        if (score > record)
            record = score;
        std::cout << "Game " << agent.n_games << " Score " << score << " Record: " << record << std::endl;

        plot_scores.push_back(score);
        total_score += score;
        float mean_score = (float)total_score / agent.n_games;
        plot_mean_scores.push_back(mean_score);
        if (showPlot)
        {
            /* plt::clf();
            plt::title("Testing...");
            plt::xlabel("Number of Games");
            plt::ylabel("Score");
            plt::plot(plot_scores);
            plt::plot(plot_mean_scores);
            plt::show(true); */
            showPlot = false;
        }
    }
}