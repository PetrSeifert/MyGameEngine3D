// #pragma once

// #include <torch/torch.h>
// #include <vector>
// #include <model.hpp>



// class Agent{
//     public:
//         int MAX_MEMORY = 100000;
//         int BATCH_SIZE = 1000;
//         float LR = 0.001;
//         int nGames = 0;
//         float epsilon = 0;
//         float gamma = .9f;
//         std::deque<std::vector<std::vector<int>>> memory{};
//         LinearQNet model = LinearQNet(11, 256, 3);
//         QTrainer trainer = QTrainer(model, LR, gamma);
//         Agent();
//         ~Agent();

//         std::vector<int> getState(FirstApp * const &game);
//         void remember(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done);
//         void trainLongMemory(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done);
//         void trainShortMemory(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done);
//         std::vector<bool> getAction();
//         void train();
// };