// #pragma once

// #include <torch/torch.h>
// #include <filesystem>

// class LinearQNet : public torch::nn::Module{
//     public:
//         torch::nn::Linear linear1 = nullptr;
//         torch::nn::Linear linear2 = nullptr;
//         LinearQNet(int inputSize, int hiddenSize, int outputSize);
//         ~LinearQNet();

//         torch::Tensor forward(torch::Tensor x);
//         void save(std::string fileName);
// };

// class QTrainer{
//     public:
//         LinearQNet model;
//         double lr;
//         float gamma;
//         torch::optim::Adam optimizer;
//         torch::nn::MSELoss criterion;
//         QTrainer(LinearQNet model, double lr, float gamma);
//         ~QTrainer();

//         void trainStep(std::vector<int> state, std::vector<int> action, std::vector<int> reward, std::vector<int> nextState, std::vector<int> done);
// };