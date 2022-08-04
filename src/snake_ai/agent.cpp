// #include <agent.hpp>

// Agent::Agent(){};
// Agent::~Agent(){};

// std::vector<int> Agent::getState(FirstApp * const &game){
//     glm::vec3 head = game->gameObjects.at(game->snakeBlocks[0]).transform.translation;
//     glm::vec3 food = game->gameObjects.at(game->foodId).transform.translation;
//     glm::vec3 pointL(head.x - 1, head.y, 0.f);
//     glm::vec3 pointR(head.x + 1, head.y, 0.f);
//     glm::vec3 pointD(head.x, head.y + 1, 0.f);
//     glm::vec3 pointU(head.x, head.y - 1, 0.f);

//     bool dirL{game->moveDir == glm::vec3(-1, 0, 0)};
//     bool dirR{game->moveDir == glm::vec3(1, 0, 0)};
//     bool dirD{game->moveDir == glm::vec3(0, 1, 0)};
//     bool dirU{game->moveDir == glm::vec3(0, -1, 0)};

//     std::vector<int> state{
//         (dirD && game->isDead(pointD) || 
//         dirU && game->isDead(pointU) || 
//         dirL && game->isDead(pointL) || 
//         dirR && game->isDead(pointR)),

//         (dirD && game->isDead(pointL) || 
//         dirU && game->isDead(pointR) || 
//         dirL && game->isDead(pointU) || 
//         dirR && game->isDead(pointD)),

//         (dirD && game->isDead(pointR) || 
//         dirU && game->isDead(pointL) || 
//         dirL && game->isDead(pointD) || 
//         dirR && game->isDead(pointU)),


//         dirR,
//         dirL,
//         dirD,
//         dirU,

//         food.x < head.x,
//         food.x > head.x, 
//         food.y < head.y, 
//         food.y > head.y 

//     };


//     return state;
// }

// void Agent::remember(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done){
//     std::vector<std::vector<int>> data{state, action, reward, nextState, done};
//     memory.insert(memory.end(), data);
// }

// void Agent::trainLongMemory(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done){
    
//     std::vector<std::vector<int>> miniSample;
//     if (memory.size() > BATCH_SIZE)
//     {
//         std::sample(memory.begin(), memory.end(), std::back_inserter(miniSample),
//                 BATCH_SIZE, std::mt19937{std::random_device{}()});
//     }
//     else
//         std::copy(memory.begin(), memory.end(), miniSample);

    
// }

// void Agent::trainShortMemory(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done){
//     trainer.trainStep(state, action, reward, nextState, done);
// }

// void Agent::train(){
//     FirstApp game;
//     while(!game.myWindow.shouldClose()){
//         std::vector<int> result = getState(&game);
//         game.run();
//     }
//     game.wait();
// }